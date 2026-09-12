#include "ui/components/ActionButton.h"
#include "ui/theme/Theme.h"

namespace sonora
{

ActionButton::ActionButton()
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ActionButton::setLabel(const juce::String& label)
{
    label_ = label;
    repaint();
}

void ActionButton::setVariant(Variant variant)
{
    variant_ = variant;
    repaint();
}

void ActionButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const bool outline = variant_ == Variant::Outline;

    if (outline)
    {
        g.setColour(hover_ ? colors::elevated() : colors::card());
        g.fillRect(bounds);
        g.setColour(hover_ ? colors::borderStrong() : colors::border());
        g.drawRect(bounds, 1);
    }
    else if (hover_)
    {
        g.setColour(colors::elevated());
        g.fillRect(bounds);
    }

    g.setColour(isEnabled() ? colors::foreground() : colors::mutedForeground());
    g.setFont(type::label(11.0f));
    g.drawFittedText(label_, bounds.reduced(10, 0), juce::Justification::centred, 1);
}

void ActionButton::mouseEnter(const juce::MouseEvent&)
{
    hover_ = true;
    repaint();
}

void ActionButton::mouseExit(const juce::MouseEvent&)
{
    hover_ = false;
    repaint();
}

void ActionButton::mouseUp(const juce::MouseEvent& event)
{
    if (isEnabled() && onClick != nullptr && getLocalBounds().contains(event.getPosition()))
        onClick();
}

} // namespace sonora
