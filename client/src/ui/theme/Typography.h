#pragma once

#include <juce_graphics/juce_graphics.h>

namespace sonora::type
{
inline juce::Font display(float height)
{
    juce::Font font(juce::FontOptions(height, juce::Font::plain));
    font.setExtraKerningFactor(0.16f);
    return font;
}

inline juce::Font label(float height)
{
    juce::Font font(juce::FontOptions(height, juce::Font::plain));
    font.setExtraKerningFactor(0.14f);
    return font;
}

inline juce::Font body(float height)
{
    return juce::Font(juce::FontOptions(height, juce::Font::plain));
}

inline juce::Font mono(float height)
{
    juce::Font font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
    return font;
}
} // namespace sonora::type
