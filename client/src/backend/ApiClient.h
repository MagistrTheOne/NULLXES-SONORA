#pragma once

#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"

#include <juce_core/juce_core.h>

#include <memory>

namespace sonora
{

class ApiClient
{
public:
    explicit ApiClient(juce::String baseUrl = "http://127.0.0.1:8000");

    bool health() const;
    juce::var analyzeFile(const juce::File& file) const;
    juce::var getAudio(const juce::String& audioId) const;
    juce::var generateReport(const juce::String& analysisId) const;
    juce::var generateHarmony(const juce::String& analysisId) const;
    juce::var getProfile() const;

    juce::String lastError() const { return lastError_; }

private:
    juce::var getJson(const juce::String& path) const;
    juce::var postJson(const juce::String& path, const juce::String& body) const;
    juce::var readJson(std::unique_ptr<juce::InputStream> stream) const;

    juce::String baseUrl_;
    mutable juce::String lastError_;
};

} // namespace sonora
