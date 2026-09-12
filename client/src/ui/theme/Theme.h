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
    g.drawFittedText(text, bounds, juce::Justification::centredLeft, 1);
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
