#pragma once

#include <optional>
#include <string>

namespace sonora::models
{

struct Insight
{
    std::string issue;
    std::string reason;
    std::string action;
    std::optional<float> frequencyHz;
    int priority = 1;
};

} // namespace sonora::models
