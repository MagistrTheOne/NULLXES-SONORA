#pragma once

#include <optional>
#include <string>
#include <vector>

namespace sonora::models
{

struct Insight
{
    std::string issue;
    std::string reason;
    std::string action;
    std::string operation;
    std::optional<float> frequencyHz;
    float confidence = 0.0f;
    int priority = 1;
};

struct EqProfile
{
    std::string operation { "Dynamic EQ" };
    float frequencyHz = 120.0f;
    float gainDb = -3.0f;
    std::string target;
};

struct Harmony
{
    std::string key;
    int bars = 8;
    std::vector<std::string> chords;
};

struct MidiClip
{
    std::string role;
    std::string key;
    int bars = 8;
    std::vector<std::string> chords;
    std::vector<std::string> notes;
    std::vector<std::string> pattern;
};

struct DropPlan
{
    std::string sectionName;
    float start = 0.0f;
    float end = 0.0f;
    std::vector<std::string> actions;
    float frequency = 3000.0f;
    float gain = 2.5f;
    float q = 1.1f;
    float energyTarget = 0.85f;
};

struct AssistOption
{
    std::string id;
    std::string label;
};

struct AssistAdvice
{
    std::string headline;
    std::string detail;
    std::vector<AssistOption> options;
    std::string provider;
};

struct ReferenceGap
{
    float loudnessLufs = 0.0f;
    float lowEnd = 0.0f;
    float stereo = 0.0f;
    float brightness = 0.0f;
};

struct ReferenceReport
{
    std::string targetFilename;
    std::string referenceFilename;
    ReferenceGap gap;
    std::vector<std::string> notes;
};

struct SessionProfile
{
    std::vector<std::string> style;
    std::vector<std::string> genres;
};

} // namespace sonora::models
