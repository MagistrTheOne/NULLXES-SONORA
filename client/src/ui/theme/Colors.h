#pragma once

#include <juce_graphics/juce_graphics.h>

namespace sonora::colors
{
// shadcn-aligned tokens: background / card / muted / border / destructive / warning
inline juce::Colour background() { return juce::Colour(0xff0b0b0b); }
inline juce::Colour card() { return juce::Colour(0xff101010); }
inline juce::Colour elevated() { return juce::Colour(0xff141414); }
inline juce::Colour foreground() { return juce::Colour(0xffe8e8e8); }
inline juce::Colour muted() { return juce::Colour(0xff8a8a8a); }
inline juce::Colour mutedForeground() { return juce::Colour(0xff6a6a6a); }
inline juce::Colour border() { return juce::Colour(0xff262626); }
inline juce::Colour borderStrong() { return juce::Colour(0xff3a3a3a); }
inline juce::Colour destructive() { return juce::Colour(0xffc45c5c); }
inline juce::Colour warning() { return juce::Colour(0xffc4a05c); }
inline juce::Colour success() { return juce::Colour(0xff9a9a9a); }
} // namespace sonora::colors
