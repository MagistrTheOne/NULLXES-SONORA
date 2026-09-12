#include "ui/components/MixHealthView.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

MixHealthView::MixHealthView(AppState& state) : state_(state) {}

void MixHealthView::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(16), "MIX HEALTH");
    bounds.removeFromTop(8);

    const auto rows = state_.mixRows();
    if (rows.empty())
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Waiting for a track");
        return;
    }

    const int rowH = juce::jmax(22, bounds.getHeight() / (int) rows.size());
    for (const auto& row : rows)
    {
        auto line = bounds.removeFromTop(rowH);
        Theme::drawMuted(g, line.removeFromLeft(78), row.name.toUpperCase());
        auto bar = line.removeFromLeft(juce::jmax(80, line.getWidth() - 110)).reduced(0, 6);
        g.setColour(colors::border());
        g.fillRect(bar);
        g.setColour(copy::toneColour(row.tone));
        g.fillRect(bar.withWidth(juce::jmax(2, juce::roundToInt((float) bar.getWidth() * row.health))));
        line.removeFromLeft(8);
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawText(juce::String(juce::roundToInt(row.health * 100.0f)), line.removeFromLeft(28),
                   juce::Justification::centredLeft, true);
        g.setColour(copy::toneColour(row.tone));
        g.drawText(row.status, line, juce::Justification::centredLeft, true);
    }
}

} // namespace sonora
