#pragma once

#include <string>
#include <vector>

namespace sonora::models
{

struct StructureSection
{
    std::string name;
    float start = 0.0f;
    float end = 0.0f;
    float energy = 0.0f;
    float bassEnergy = 0.0f;
    float transientDensity = 0.0f;
    float stereoWidth = 0.0f;
};

struct MixAxis
{
    float sub = 0.0f;
    float low = 0.0f;
    float control = 0.0f;
    float value = 0.0f;
    std::string risk;
    std::string why;
    std::string method;
};

struct TranslationTarget
{
    std::string name;
    float score = 0.0f;
    std::string issue;
    std::string reason;
    std::string action;
};

struct SonoraObject
{
    std::string type;
    std::string input;
    std::string status;
    float frequency = 0.0f;
    float gain = 0.0f;
    float q = 0.0f;
};

struct TrackDna
{
    float tempo = 0.0f;
    std::string keyName;
    float keyConfidence = 0.0f;
    std::vector<std::string> genreProfile;
    std::vector<float> energyCurve;
    std::vector<float> energyPeaks;
    float energyMean = 0.0f;
    float energyPeak = 0.0f;
    std::vector<StructureSection> sections;
    MixAxis lowEnd;
    MixAxis brightness;
    MixAxis stereo;
    MixAxis dynamics;
    std::vector<TranslationTarget> translation;
    std::vector<std::string> maskingRoles;
    std::vector<std::vector<float>> maskingMatrix;
    std::vector<SonoraObject> objects;
};

} // namespace sonora::models
