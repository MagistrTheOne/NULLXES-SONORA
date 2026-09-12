#include "backend/ApiClient.h"

#include "backend/Dto.h"

namespace sonora
{

ApiClient::ApiClient(juce::String baseUrl)
    : baseUrl_(std::move(baseUrl))
{
    if (baseUrl_.endsWithChar('/'))
        baseUrl_ = baseUrl_.dropLastCharacters(1);
}

juce::var ApiClient::readJson(std::unique_ptr<juce::InputStream> stream) const
{
    if (stream == nullptr)
        return {};
    const auto text = stream->readEntireStreamAsString();
    auto parsed = juce::JSON::parse(text);
    if (parsed.isVoid())
        lastError_ = text.isEmpty() ? "Empty response" : text.substring(0, 180);
    return parsed;
}

juce::var ApiClient::getJson(const juce::String& path) const
{
    lastError_.clear();
    int status = 0;
    juce::URL url(baseUrl_ + path);
    auto stream = url.createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
            .withConnectionTimeoutMs(15000)
            .withNumRedirectsToFollow(2)
            .withStatusCode(&status));
    if (stream == nullptr)
    {
        lastError_ = "Backend unreachable";
        return {};
    }
    auto json = readJson(std::move(stream));
    if (status >= 400)
    {
        lastError_ = dto::parseError(json);
        if (lastError_.isEmpty())
            lastError_ = "HTTP " + juce::String(status);
        return {};
    }
    return json;
}

juce::var ApiClient::postJson(const juce::String& path, const juce::String& body) const
{
    lastError_.clear();
    int status = 0;
    juce::URL url(baseUrl_ + path);
    auto stream = url.withPOSTData(body).createInputStream(
        juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\n")
            .withHttpRequestCmd("POST")
            .withConnectionTimeoutMs(120000)
            .withNumRedirectsToFollow(2)
            .withStatusCode(&status));
    if (stream == nullptr)
    {
        lastError_ = "Backend request failed";
        return {};
    }
    auto json = readJson(std::move(stream));
    if (status >= 400)
    {
        lastError_ = dto::parseError(json);
        if (lastError_.isEmpty())
            lastError_ = "HTTP " + juce::String(status);
        return {};
    }
    return json;
}

bool ApiClient::health() const
{
    auto json = getJson("/health");
    return dto::parseHealthOk(json);
}

juce::var ApiClient::analyzeFile(const juce::File& file) const
{
    lastError_.clear();
    const auto ext = file.getFileExtension().toLowerCase();
    juce::String mime = "application/octet-stream";
    if (ext == ".wav")
        mime = "audio/wav";
    else if (ext == ".mp3")
        mime = "audio/mpeg";
    else if (ext == ".flac")
        mime = "audio/flac";

    int status = 0;
    juce::URL url(baseUrl_ + "/api/v1/audio/analyze");
    auto stream = url.withFileToUpload("file", file, mime)
                      .createInputStream(
                          juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                              .withHttpRequestCmd("POST")
                              .withConnectionTimeoutMs(180000)
                              .withNumRedirectsToFollow(2)
                              .withStatusCode(&status));
    if (stream == nullptr)
    {
        lastError_ = "Upload failed";
        return {};
    }
    auto json = readJson(std::move(stream));
    if (status >= 400)
    {
        lastError_ = dto::parseError(json);
        if (lastError_.isEmpty())
            lastError_ = "HTTP " + juce::String(status);
        return {};
    }
    return json;
}

juce::var ApiClient::getAudio(const juce::String& audioId) const
{
    return getJson("/api/v1/audio/" + audioId);
}

juce::var ApiClient::generateReport(const juce::String& analysisId) const
{
    return postJson(
        "/api/v1/recommendation/generate",
        R"({"analysis_id":")" + analysisId + R"("})");
}

juce::var ApiClient::generateHarmony(const juce::String& analysisId) const
{
    return postJson(
        "/api/v1/generate/harmony",
        R"({"analysis_id":")" + analysisId + R"("})");
}

juce::var ApiClient::getProfile() const
{
    return getJson("/api/v1/profile");
}

} // namespace sonora
