#pragma once

#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"

#include <juce_core/juce_core.h>

namespace sonora::dto
{

bool parseHealthOk(const juce::var& json);
juce::String parseAudioId(const juce::var& json);
juce::String parseAnalysisId(const juce::var& json);
juce::String parseAnalysisStatus(const juce::var& json);
bool parseCompletedAnalysis(
    const juce::var& json,
    models::AudioAnalysis& analysis,
    std::vector<models::Issue>& issues);
std::vector<models::Insight> parseInsights(const juce::var& json);
models::Harmony parseHarmony(const juce::var& json);
models::SessionProfile parseProfile(const juce::var& json);
juce::String parseError(const juce::var& json);

} // namespace sonora::dto
