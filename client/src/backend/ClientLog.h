#pragma once

#include <juce_core/juce_core.h>

namespace sonora
{

inline void clientLog(const juce::String& line)
{
    const auto stamped = juce::Time::getCurrentTime().toString(true, true, true, true) + "  " + line;
    juce::Logger::writeToLog(stamped);
    DBG("[SONORA] " << line);
}

} // namespace sonora
