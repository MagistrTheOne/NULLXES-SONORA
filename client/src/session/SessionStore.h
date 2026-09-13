#pragma once

#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"
#include "soni/SoniTypes.h"

#include <juce_core/juce_core.h>

#include <optional>
#include <vector>

namespace sonora::session
{

struct Blob
{
    juce::String version;
    juce::String name;
    juce::String tab { "listen" };
    bool met = false;
    bool understood = false;
    bool hasAnalysis = false;
    models::AudioAnalysis analysis;
    std::vector<models::Issue> issues;
    std::optional<models::Harmony> harmony;
    std::optional<models::MidiClip> bass;
    std::optional<models::MidiClip> pad;
    std::optional<models::DropPlan> drop;
    std::optional<models::EqProfile> eq;
    std::vector<soni::Message> soni;
};

juce::File defaultFile();
juce::String encode(const Blob& blob);
bool decode(const juce::String& json, Blob& blob);
bool save(const juce::String& json);
juce::String load();

} // namespace sonora::session
