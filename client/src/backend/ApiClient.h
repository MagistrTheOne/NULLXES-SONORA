#pragma once

#include "backend/Fault.h"
#include "backend/HttpTransport.h"

#include <juce_core/juce_core.h>

namespace sonora
{

class ApiClient
{
public:
    ApiClient();

    const juce::String& baseUrl() const { return baseUrl_; }
    const Fault& lastFault() const { return lastFault_; }

    bool health() const;
    juce::var analyzeFile(const juce::File& file) const;
    juce::var getAudio(const juce::String& audioId) const;
    juce::var generateReport(const juce::String& analysisId) const;
    juce::var generateHarmony(const juce::String& analysisId) const;
    juce::var generateBass(const juce::String& analysisId) const;
    juce::var generatePad(const juce::String& analysisId) const;
    juce::var generateDrop(const juce::String& analysisId) const;
    juce::var requestAssist(const juce::String& analysisId) const;
    juce::var compareReference(const juce::String& audioId, const juce::File& file) const;
    juce::var getProfile() const;

private:
    juce::var accept(const HttpResult& result, const juce::String& title) const;
    juce::String mimeFor(const juce::File& file) const;

    juce::String baseUrl_;
    mutable Fault lastFault_;
};

} // namespace sonora
