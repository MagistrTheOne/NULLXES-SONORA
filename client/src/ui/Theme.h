#pragma once

#include <juce_graphics/juce_graphics.h>

namespace sonora::theme
{
inline juce::Colour background() { return juce::Colour(0xff0a0a0a); }
inline juce::Colour surface() { return juce::Colour(0xff111111); }
inline juce::Colour text() { return juce::Colour(0xffe8e8e8); }
inline juce::Colour muted() { return juce::Colour(0xff8a8a8a); }
inline juce::Colour hairline() { return juce::Colour(0xff2a2a2a); }
inline juce::Colour severityHigh() { return juce::Colour(0xffc45c5c); }
inline juce::Colour severityMid() { return juce::Colour(0xffc4a05c); }

inline juce::Font displayFont(float height)
{
    juce::Font font(juce::FontOptions(height, juce::Font::plain));
    font.setExtraKerningFactor(0.18f);
    return font;
}

inline juce::Font labelFont(float height)
{
    juce::Font font(juce::FontOptions(height, juce::Font::plain));
    font.setExtraKerningFactor(0.12f);
    return font;
}
} // namespace sonora::theme
