#pragma once

#include <juce_graphics/juce_graphics.h>

namespace sonora::type
{
inline juce::String family()
{
#if JUCE_WINDOWS
    return "Segoe UI";
#else
    return juce::Font::getDefaultSansSerifFontName();
#endif
}

inline juce::Font font(float height, int style = juce::Font::plain)
{
    return juce::Font(juce::FontOptions(family(), height, style));
}

inline juce::Font display(float height)
{
    return font(height);
}

inline juce::Font label(float height)
{
    return font(height);
}

inline juce::Font body(float height)
{
    return font(height);
}

inline juce::Font mono(float height)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}
} // namespace sonora::type
