#pragma once

#include <juce_core/juce_core.h>

namespace sonora
{

struct Fault
{
    juce::String title;
    int httpStatus = 0;
    juce::String reason;

    bool empty() const { return title.isEmpty() && reason.isEmpty(); }

    juce::String httpLabel() const
    {
        return httpStatus > 0 ? juce::String(httpStatus) : "-";
    }
};

} // namespace sonora
