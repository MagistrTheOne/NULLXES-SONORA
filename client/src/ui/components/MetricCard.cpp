#include "ui/components/MetricCard.h"
#include "ui/theme/Theme.h"

namespace sonora
{

MetricCard::MetricCard()
{
    setOpaque(false);
}

void MetricCard::setLabel(const juce::String& label)
{
    label_ = label;
    repaint();
}

void MetricCard::setValue(const juce::String& value)
{
    value_ = value;
    empty_ = value == "---" || value.isEmpty();
    repaint();
}

void MetricCard::setHint(const juce::String& hint)
{
    hint_ = hint;
    repaint();
}

void MetricCard::setEmpty(bool empty)
{
    empty_ = empty;
    if (empty)
        value_ = "---";
    repaint();
}

void MetricCard::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(16, 14);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawText(label_, bounds.removeFromTop(16), juce::Justification::centredLeft, true);

    bounds.removeFromTop(6);
    g.setColour(empty_ ? colors::mutedForeground() : colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText(value_, bounds.removeFromTop(28), juce::Justification::centredLeft, true);

    if (hint_.isNotEmpty())
    {
        bounds.removeFromTop(4);
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(11.0f));
        g.drawText(hint_, bounds, juce::Justification::centredLeft, true);
    }
}

} // namespace sonora
