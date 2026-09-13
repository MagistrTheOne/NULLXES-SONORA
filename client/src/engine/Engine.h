#pragma once

#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <optional>
#include <vector>

namespace sonora::engine
{

inline constexpr const char* kAnalyzerVersion = "SONORA_DSP_v1.0";

struct Result
{
    models::AudioAnalysis analysis;
    std::vector<models::Issue> issues;
    juce::String error;
    bool ok() const { return error.isEmpty(); }
};

bool loadFile(const juce::File& file, juce::AudioBuffer<float>& buffer, double& sampleRate, juce::String& error);

Result analyze(const juce::AudioBuffer<float>& buffer, double sampleRate);

models::Harmony makeHarmony(const models::AudioAnalysis& analysis);
models::MidiClip makeBass(const models::AudioAnalysis& analysis);
models::MidiClip makePad(const models::AudioAnalysis& analysis);
models::DropPlan makeDrop(const models::AudioAnalysis& analysis);
models::AssistAdvice makeAssist(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues);
bool writeMidiFile(
    const juce::File& file,
    const std::optional<models::Harmony>& harmony,
    const std::optional<models::MidiClip>& bass,
    const std::optional<models::MidiClip>& pad,
    juce::String& error);

models::ReferenceReport compare(
    const models::AudioAnalysis& target,
    const models::AudioAnalysis& reference,
    const juce::String& targetName,
    const juce::String& referenceName);

} // namespace sonora::engine
