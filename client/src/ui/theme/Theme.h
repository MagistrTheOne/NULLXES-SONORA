#pragma once

#include "ui/theme/Colors.h"
#include "ui/theme/Typography.h"

#include <juce_graphics/juce_graphics.h>

namespace sonora::theme
{
inline juce::Colour background() { return colors::background(); }

inline void fillCard(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(colors::card());
    g.fillRect(bounds);
    g.setColour(colors::border());
    g.drawRect(bounds, 1);
}

inline void drawSectionLabel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text)
{
    g.setColour(colors::muted());
    g.setFont(type::label(11.0f));
    g.drawText(text, bounds, juce::Justification::centredLeft, true);
}

inline juce::Colour severityColour(float severity)
{
    if (severity >= 0.65f)
        return colors::destructive();
    if (severity >= 0.40f)
        return colors::warning();
    return colors::muted();
}
} // namespace sonora::theme

namespace sonora::Theme
{
inline juce::Colour card() { return colors::card(); }
inline juce::Colour surface() { return colors::elevated(); }
inline juce::Colour text() { return colors::foreground(); }
inline juce::Colour muted() { return colors::mutedForeground(); }
inline juce::Colour accent() { return colors::foreground(); }
inline juce::Colour border() { return colors::border(); }

inline void drawLabel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text)
{
    theme::drawSectionLabel(g, bounds, text);
}

inline void drawBody(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text)
{
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawText(text, bounds, juce::Justification::centredLeft, true);
}

inline void drawMuted(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text)
{
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawText(text, bounds, juce::Justification::centredLeft, true);
}
} // namespace sonora::Theme
