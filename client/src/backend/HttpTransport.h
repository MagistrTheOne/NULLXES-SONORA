#pragma once

#include <juce_core/juce_core.h>

namespace sonora
{

struct HttpResult
{
    int status = 0;
    juce::String body;
    juce::String transportError;
};

class HttpTransport
{
public:
    HttpResult get(const juce::String& url, int timeoutMs) const;
    HttpResult postJson(const juce::String& url, const juce::String& json, int timeoutMs) const;
    HttpResult postMultipartFile(
        const juce::String& url,
        const juce::String& fieldName,
        const juce::String& filename,
        const juce::String& mime,
        const juce::MemoryBlock& data,
        int timeoutMs,
        const juce::StringPairArray& extraFields = {}) const;

private:
    HttpResult execute(
        const juce::String& method,
        const juce::String& url,
        const juce::String& extraHeaders,
        const void* body,
        size_t bodySize,
        int timeoutMs) const;
};

} // namespace sonora
