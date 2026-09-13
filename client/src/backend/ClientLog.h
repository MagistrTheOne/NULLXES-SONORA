#pragma once

#include <juce_core/juce_core.h>

namespace sonora
{

inline void clientLog(const juce::String& line)
{
#if JUCE_DEBUG
    DBG("[SONORA] " << line);
#else
    juce::ignoreUnused(line);
#endif
}

} // namespace sonora
