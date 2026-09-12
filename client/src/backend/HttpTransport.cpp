#include "backend/HttpTransport.h"

#include "backend/ClientLog.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>

#include <cstring>
#include <string>
#include <vector>

namespace sonora
{
namespace
{
std::wstring wide(const juce::String& text)
{
    return std::wstring(text.toWideCharPointer());
}

juce::String statusFromWinHttp(DWORD error)
{
    switch (error)
    {
        case ERROR_WINHTTP_CANNOT_CONNECT:
        case ERROR_WINHTTP_CONNECTION_ERROR:
            return "connection refused";
        case ERROR_WINHTTP_TIMEOUT:
            return "timeout";
        case ERROR_WINHTTP_NAME_NOT_RESOLVED:
            return "host not found";
        case ERROR_WINHTTP_INVALID_SERVER_RESPONSE:
            return "invalid server response";
        case ERROR_WINHTTP_INVALID_URL:
            return "invalid URL";
        default:
            break;
    }
    if (error == 10061)
        return "connection refused";
    return "winhttp " + juce::String((int) error);
}

struct UrlParts
{
    bool https = false;
    std::wstring host;
    INTERNET_PORT port = 80;
    std::wstring path;
};

bool crackUrl(const juce::String& url, UrlParts& parts, juce::String& error)
{
    juce::URL parsed(url);
    if (!parsed.isWellFormed())
    {
        error = "invalid URL";
        return false;
    }

    parts.https = parsed.getScheme().equalsIgnoreCase("https");
    parts.host = wide(parsed.getDomain());
    const int port = parsed.getPort();
    parts.port = (INTERNET_PORT) (port > 0 ? port : (parts.https ? 443 : 80));

    juce::String path = parsed.getSubPath();
    if (path.isEmpty())
        path = "/";
    if (!path.startsWithChar('/'))
        path = "/" + path;
    const auto query = parsed.getQueryString();
    if (query.isNotEmpty())
        path += query.startsWithChar('?') ? query : ("?" + query);
    parts.path = wide(path);
    return true;
}

class WinHandle
{
public:
    WinHandle() = default;
    explicit WinHandle(HINTERNET handle) : handle_(handle) {}
    ~WinHandle() { close(); }

    WinHandle(const WinHandle&) = delete;
    WinHandle& operator=(const WinHandle&) = delete;

    WinHandle(WinHandle&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    WinHandle& operator=(WinHandle&& other) noexcept
    {
        if (this != &other)
        {
            close();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    HINTERNET get() const { return handle_; }
    explicit operator bool() const { return handle_ != nullptr; }

private:
    void close()
    {
        if (handle_ != nullptr)
        {
            WinHttpCloseHandle(handle_);
            handle_ = nullptr;
        }
    }

    HINTERNET handle_ = nullptr;
};
} // namespace

HttpResult HttpTransport::get(const juce::String& url, int timeoutMs) const
{
    return execute("GET", url, "Accept: application/json\r\n", nullptr, 0, timeoutMs);
}

HttpResult HttpTransport::postJson(const juce::String& url, const juce::String& json, int timeoutMs) const
{
    const auto utf8 = json.toRawUTF8();
    const auto bytes = strlen(utf8);
    return execute(
        "POST",
        url,
        "Content-Type: application/json\r\nAccept: application/json\r\n",
        utf8,
        bytes,
        timeoutMs);
}

HttpResult HttpTransport::postMultipartFile(
    const juce::String& url,
    const juce::String& fieldName,
    const juce::String& filename,
    const juce::String& mime,
    const juce::MemoryBlock& data,
    int timeoutMs,
    const juce::StringPairArray& extraFields) const
{
    const auto boundary = "----SonoraBoundary" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
    auto asciiName = filename;
    const bool ascii = asciiName.containsOnly(
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._- ");
    if (!ascii)
    {
        const auto ext = filename.fromLastOccurrenceOf(".", true, false);
        asciiName = "track" + (ext.isNotEmpty() ? ext : ".bin");
    }
    asciiName = asciiName.replaceCharacter('"', '_');

    juce::MemoryOutputStream body;
    for (int i = 0; i < extraFields.size(); ++i)
    {
        body << "--" << boundary << "\r\n";
        body << "Content-Disposition: form-data; name=\"" << extraFields.getAllKeys()[i] << "\"\r\n\r\n";
        body << extraFields.getAllValues()[i] << "\r\n";
    }
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"" << fieldName
         << "\"; filename=\"" << asciiName << "\"";
    if (!ascii)
        body << "; filename*=UTF-8''" << juce::URL::addEscapeChars(filename, true);
    body << "\r\n";
    body << "Content-Type: " << mime << "\r\n\r\n";
    body.write(data.getData(), data.getSize());
    body << "\r\n--" << boundary << "--\r\n";

    const auto headers = "Content-Type: multipart/form-data; boundary=" + boundary
                         + "\r\nAccept: application/json\r\n";
    return execute("POST", url, headers, body.getData(), (size_t) body.getDataSize(), timeoutMs);
}

HttpResult HttpTransport::execute(
    const juce::String& method,
    const juce::String& url,
    const juce::String& extraHeaders,
    const void* body,
    size_t bodySize,
    int timeoutMs) const
{
    HttpResult result;
    UrlParts parts;
    if (!crackUrl(url, parts, result.transportError))
        return result;

    WinHandle session(WinHttpOpen(
        L"SONORA/0.4",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0));
    if (!session)
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    WinHttpSetTimeouts(session.get(), timeoutMs, juce::jmin(timeoutMs, 8000), timeoutMs, timeoutMs);

    WinHandle connect(WinHttpConnect(session.get(), parts.host.c_str(), parts.port, 0));
    if (!connect)
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    const DWORD flags = parts.https ? WINHTTP_FLAG_SECURE : 0;
    WinHandle request(WinHttpOpenRequest(
        connect.get(),
        wide(method).c_str(),
        parts.path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags));
    if (!request)
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    const auto headers = wide(extraHeaders);
    if (!WinHttpAddRequestHeaders(
            request.get(),
            headers.c_str(),
            (DWORD) -1,
            WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    const auto sent = WinHttpSendRequest(
        request.get(),
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        (LPVOID) body,
        (DWORD) bodySize,
        (DWORD) bodySize,
        0);
    if (!sent)
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    if (!WinHttpReceiveResponse(request.get(), nullptr))
    {
        result.transportError = statusFromWinHttp(GetLastError());
        return result;
    }

    DWORD status = 0;
    DWORD statusSize = sizeof(status);
    if (WinHttpQueryHeaders(
            request.get(),
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &status,
            &statusSize,
            WINHTTP_NO_HEADER_INDEX))
    {
        result.status = (int) status;
    }

    std::vector<char> buffer;
    for (;;)
    {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request.get(), &available))
        {
            result.transportError = statusFromWinHttp(GetLastError());
            return result;
        }
        if (available == 0)
            break;

        const auto offset = buffer.size();
        buffer.resize(offset + available);
        DWORD read = 0;
        if (!WinHttpReadData(request.get(), buffer.data() + offset, available, &read))
        {
            result.transportError = statusFromWinHttp(GetLastError());
            return result;
        }
        buffer.resize(offset + read);
        if (read == 0)
            break;
    }

    if (!buffer.empty())
        result.body = juce::String::fromUTF8(buffer.data(), (int) buffer.size());
    return result;
}

} // namespace sonora
