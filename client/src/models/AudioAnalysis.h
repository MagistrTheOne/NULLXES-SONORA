#pragma once

#include <optional>
#include <string>

namespace sonora::models
{

struct KeyEstimation
{
    std::optional<std::string> key;
    float confidence = 0.0f;
    std::string method;
};

struct FrequencyDistribution
{
    float sub = 0.0f;
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
    float air = 0.0f;
};

struct AudioAnalysis
{
    std::string analyzerVersion;
    float bpm = 0.0f;
    float durationSec = 0.0f;
    int sampleRate = 0;
    int channels = 0;
    float loudnessLufsApprox = 0.0f;
    float dynamicRangeDb = 0.0f;
    float stereoWidth = 0.0f;
    KeyEstimation key;
    FrequencyDistribution bands;
};

} // namespace sonora::models
