#pragma once

#include <juce_graphics/juce_graphics.h>

namespace sonora::type
{
inline juce::Font display(float height)
{
    return juce::Font(juce::FontOptions(height, juce::Font::plain));
}

inline juce::Font label(float height)
{
    return juce::Font(juce::FontOptions(height, juce::Font::plain));
}

inline juce::Font body(float height)
{
    return juce::Font(juce::FontOptions(height, juce::Font::plain));
}

inline juce::Font mono(float height)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}
} // namespace sonora::type
