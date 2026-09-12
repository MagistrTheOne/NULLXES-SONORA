#pragma once

#include <string>

namespace sonora::models
{

struct Issue
{
    std::string type;
    float severity = 0.0f;
    std::string area;
    std::string detail;
};

} // namespace sonora::models
