#pragma once

#include "models/AudioAnalysis.h"
#include "models/Issue.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

namespace sonora::copy
{

enum class Tone
{
    Good,
    Neutral,
    Attention
};

struct MixRow
{
    juce::String name;
    float health = 0.0f;
    juce::String status;
    Tone tone = Tone::Neutral;
};

struct Finding
{
    juce::String headline;
    juce::String detail;
};

struct Delta
{
    bool valid = false;
    float loudness = 0.0f;
    float lowEnd = 0.0f;
    float stereo = 0.0f;
    float energy = 0.0f;
};

juce::Colour toneColour(Tone tone);
juce::String riskStatus(const juce::String& risk);
Tone riskTone(const juce::String& risk);
juce::String issuePhrase(const models::Issue& issue);
juce::String formatTime(float seconds);
juce::String styleLine(const models::TrackDna* dna, const std::vector<juce::String>& profile);
std::vector<juce::String> moodTags(const models::AudioAnalysis& analysis);
int healthScore(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues);
juce::String healthVerdict(int score);
std::vector<MixRow> mixRows(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues);
Finding assistFinding(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues);
Delta mixDelta(const models::AudioAnalysis& previous, const models::AudioAnalysis& current);
juce::String signedPercent(float value);

} // namespace sonora::copy
