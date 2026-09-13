#include "ui/components/SonicIdentity.h"

#include "ui/theme/Theme.h"

namespace sonora
{

SonicIdentity::SonicIdentity(AppState& state) : state_(state) {}

void SonicIdentity::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(14), "SONIC IDENTITY");
    bounds.removeFromTop(8);

    const auto axes = state_.sonicIdentity();
    if (axes.empty())
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Load a track. SONORA will listen.");
        return;
    }

    const int rowH = juce::jmax(20, bounds.getHeight() / (int) axes.size());
    for (const auto& axis : axes)
    {
        auto row = bounds.removeFromTop(rowH);
        Theme::drawBody(g, row.removeFromLeft(118), axis.name);
        auto bar = row.removeFromLeft(juce::jmax(80, row.getWidth() - 44)).reduced(0, juce::jmax(4, row.getHeight() / 3));
        Theme::drawBlocks(g, bar, axis.value);
        row.removeFromLeft(8);
        g.setColour(colors::mutedForeground());
        g.setFont(type::label(10.0f));
        g.drawText(juce::String(juce::roundToInt(axis.value * 100.0f)), row, juce::Justification::centredLeft, true);
    }
}

} // namespace sonora
