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

struct SessionProfile
{
    std::vector<std::string> style;
    std::vector<std::string> genres;
};

} // namespace sonora::models
