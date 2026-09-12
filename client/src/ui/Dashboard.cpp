#include "ui/Dashboard.h"
#include "ui/Theme.h"

namespace sonora
{

Dashboard::Dashboard(AppState& state)
    : state_(state)
{
    setOpaque(true);
}

void Dashboard::paint(juce::Graphics& g)
{
    g.fillAll(theme::background());

    auto bounds = getLocalBounds().reduced(48);

    g.setColour(theme::muted());
    g.setFont(theme::labelFont(13.0f));
    g.drawFittedText("NULLXES", bounds.removeFromTop(22), juce::Justification::centred, 1);

    bounds.removeFromTop(8);

    g.setColour(theme::text());
    g.setFont(theme::displayFont(42.0f));
    g.drawFittedText("SONORA", bounds.removeFromTop(56), juce::Justification::centred, 1);

    bounds.removeFromTop(12);

    g.setColour(theme::hairline());
    g.fillRect(bounds.removeFromTop(1).reduced(120, 0));

    bounds.removeFromTop(18);

    g.setColour(theme::muted());
    g.setFont(theme::labelFont(12.0f));
    g.drawFittedText(
        "ADAPTIVE SOUND INTELLIGENCE",
        bounds.removeFromTop(20),
        juce::Justification::centred,
        1);

    juce::ignoreUnused(state_);
}

void Dashboard::resized() {}

} // namespace sonora
