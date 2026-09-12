#pragma once

#include <optional>
#include <string>

namespace sonora::models
{

struct Insight
{
    std::string action;
    std::string target;
    std::optional<float> frequencyHz;
    std::string rationale;
    int priority = 1;
};

} // namespace sonora::models
