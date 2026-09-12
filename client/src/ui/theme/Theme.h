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

inline void drawBlocks(juce::Graphics& g, juce::Rectangle<int> bounds, float value, int slots = 10)
{
    const int filled = juce::jlimit(0, slots, juce::roundToInt(juce::jlimit(0.0f, 1.0f, value) * (float) slots));
    const int gap = 3;
    const int w = juce::jmax(2, (bounds.getWidth() - gap * (slots - 1)) / slots);
    for (int i = 0; i < slots; ++i)
    {
        g.setColour(i < filled ? colors::foreground() : colors::border());
        g.fillRect(bounds.getX() + i * (w + gap), bounds.getY(), w, bounds.getHeight());
    }
}

inline juce::Colour riskColour(const juce::String& risk)
{
    if (risk == "high")
        return colors::destructive();
    if (risk == "medium")
        return colors::warning();
    return colors::mutedForeground();
}
} // namespace sonora::Theme
