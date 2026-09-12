#include "backend/ApiClient.h"

#include "backend/ClientLog.h"
#include "backend/Dto.h"

namespace sonora
{
namespace
{
juce::String resolveBaseUrl()
{
    auto url = juce::SystemStats::getEnvironmentVariable("SONORA_API_URL", "http://127.0.0.1:8000").trim();
    if (url.endsWithChar('/'))
        url = url.dropLastCharacters(1);
    return url;
}

juce::String clipBody(const juce::String& body)
{
    return body.substring(0, 400);
}

juce::String reasonFromHttp(int status, const juce::var& json)
{
    auto parsed = dto::parseError(json);
    if (parsed.isNotEmpty())
        return parsed;
    if (status == 422)
        return "validation error";
    if (status == 413)
        return "file too large";
    if (status == 415)
        return "unsupported media type";
    if (status >= 500)
        return "backend error";
    if (status > 0)
        return "HTTP " + juce::String(status);
    return "unknown error";
}
} // namespace

ApiClient::ApiClient()
    : baseUrl_(resolveBaseUrl())
{
    clientLog("ApiClient baseUrl=" + baseUrl_);
}

juce::String ApiClient::mimeFor(const juce::File& file) const
{
    const auto ext = file.getFileExtension().toLowerCase();
    if (ext == ".wav")
        return "audio/wav";
    if (ext == ".mp3")
        return "audio/mpeg";
    if (ext == ".flac")
        return "audio/flac";
    return "application/octet-stream";
}

juce::var ApiClient::accept(const HttpResult& result, const juce::String& title) const
{
    lastFault_ = {};
    clientLog(title + " HTTP=" + juce::String(result.status)
              + " transport=" + (result.transportError.isEmpty() ? "-" : result.transportError)
              + " body=" + clipBody(result.body));

    if (result.transportError.isNotEmpty())
    {
        lastFault_.title = title + " FAILED";
        lastFault_.httpStatus = result.status;
        lastFault_.reason = result.transportError;
        return {};
    }

    auto json = juce::JSON::parse(result.body);
    if (result.status >= 400)
    {
        lastFault_.title = title + " FAILED";
        lastFault_.httpStatus = result.status;
        lastFault_.reason = reasonFromHttp(result.status, json);
        return {};
    }

    if (result.body.isEmpty())
    {
        lastFault_.title = title + " FAILED";
        lastFault_.httpStatus = result.status;
        lastFault_.reason = "empty response";
        return {};
    }

    if (json.isVoid())
    {
        lastFault_.title = title + " FAILED";
        lastFault_.httpStatus = result.status;
        lastFault_.reason = "invalid JSON";
        return {};
    }

    return json;
}

bool ApiClient::health() const
{
    const auto url = baseUrl_ + "/health";
    clientLog("GET " + url);
    auto json = accept(HttpTransport().get(url, 8000), "HEALTH");
    return dto::parseHealthOk(json);
}

juce::var ApiClient::analyzeFile(const juce::File& file) const
{
    lastFault_ = {};
    const auto path = file.getFullPathName();
    const auto name = file.getFileName();
    const auto size = file.getSize();
    const auto mime = mimeFor(file);
    const auto url = baseUrl_ + "/api/v1/audio/analyze";
    const auto ext = file.getFileExtension().toLowerCase();

    clientLog("LOAD TRACK path=" + path);
    clientLog("filename=" + name + " size=" + juce::String(size) + " mime=" + mime);
    clientLog("POST " + url);

    if (!file.existsAsFile())
    {
        lastFault_ = { "UPLOAD FAILED", 0, "file not found" };
        clientLog("UPLOAD FAILED reason=file not found");
        return {};
    }
    if (size <= 0)
    {
        lastFault_ = { "UPLOAD FAILED", 0, "file is empty" };
        clientLog("UPLOAD FAILED reason=file is empty");
        return {};
    }
    if (ext != ".wav" && ext != ".mp3" && ext != ".flac")
    {
        lastFault_ = { "UPLOAD FAILED", 0, "unsupported format (wav/mp3/flac)" };
        clientLog("UPLOAD FAILED reason=unsupported format");
        return {};
    }

    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.getSize() == 0)
    {
        lastFault_ = { "UPLOAD FAILED", 0, "cannot read file" };
        clientLog("UPLOAD FAILED reason=cannot read file");
        return {};
    }

    const auto result = HttpTransport().postMultipartFile(url, "file", name, mime, data, 180000);
    return accept(result, "UPLOAD");
}

juce::var ApiClient::getAudio(const juce::String& audioId) const
{
    const auto url = baseUrl_ + "/api/v1/audio/" + audioId;
    clientLog("GET " + url);
    return accept(HttpTransport().get(url, 15000), "REQUEST");
}

juce::var ApiClient::generateReport(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/recommendation/generate";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "REPORT");
}

juce::var ApiClient::generateHarmony(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/generate/harmony";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "HARMONY");
}

juce::var ApiClient::generateBass(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/generate/bass";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "BASS");
}

juce::var ApiClient::generatePad(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/generate/pad";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "PAD");
}

juce::var ApiClient::generateDrop(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/generate/drop";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "DROP");
}

juce::var ApiClient::requestAssist(const juce::String& analysisId) const
{
    const auto url = baseUrl_ + "/api/v1/assist";
    const auto body = R"({"analysis_id":")" + analysisId + R"("})";
    clientLog("POST " + url + " body=" + body);
    return accept(HttpTransport().postJson(url, body, 120000), "ASSIST");
}

juce::var ApiClient::compareReference(const juce::String& audioId, const juce::File& file) const
{
    lastFault_ = {};
    const auto url = baseUrl_ + "/api/v1/audio/compare";
    const auto name = file.getFileName();
    const auto mime = mimeFor(file);
    clientLog("POST " + url + " audio_id=" + audioId + " file=" + name);

    if (!file.existsAsFile() || file.getSize() <= 0)
    {
        lastFault_ = { "REFERENCE FAILED", 0, "reference file missing" };
        return {};
    }

    juce::MemoryBlock data;
    if (!file.loadFileAsData(data) || data.getSize() == 0)
    {
        lastFault_ = { "REFERENCE FAILED", 0, "cannot read reference" };
        return {};
    }

    juce::StringPairArray fields;
    fields.set("audio_id", audioId);
    return accept(
        HttpTransport().postMultipartFile(url, "file", name, mime, data, 180000, fields),
        "REFERENCE");
}

juce::var ApiClient::getProfile() const
{
    const auto url = baseUrl_ + "/api/v1/profile";
    clientLog("GET " + url);
    return accept(HttpTransport().get(url, 8000), "PROFILE");
}

} // namespace sonora
