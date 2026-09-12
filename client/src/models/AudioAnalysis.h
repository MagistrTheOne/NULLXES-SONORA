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
};

} // namespace sonora::models
