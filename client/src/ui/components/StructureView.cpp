#include "ui/components/StructureView.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

StructureView::StructureView(AppState& state) : state_(state)
{
    setOpaque(false);
}

void StructureView::paint(juce::Graphics& g)
{
    const auto* dna = state_.dna();
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(20, 16);
    Theme::drawMuted(g, bounds.removeFromTop(14), "ARRANGEMENT");
    bounds.removeFromTop(8);
    g.setColour(colors::foreground());
    g.setFont(type::display(20.0f));
    g.drawText("Form of the track", bounds.removeFromTop(26), juce::Justification::centredLeft, true);
    bounds.removeFromTop(10);

    if (dna == nullptr || dna->sections.empty() || !state_.analysis())
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Load a track. SONORA will hear the shape.");
        return;
    }

    const float duration = juce::jmax(0.1f, state_.analysis()->durationSec);
    auto lane = bounds.removeFromTop(36);
    int x = lane.getX();
    for (const auto& section : dna->sections)
    {
        const float span = juce::jmax(0.0f, section.end - section.start) / duration;
        const int w = juce::jmax(6, juce::roundToInt((float) lane.getWidth() * span));
        auto slice = juce::Rectangle<int>(x, lane.getY(), juce::jmin(w, lane.getRight() - x), lane.getHeight());
        g.setColour(section.name == "drop" ? colors::destructive()
                                           : colors::foreground().withAlpha(0.16f + section.energy * 0.5f));
        g.fillRect(slice);
        x += w;
    }
    bounds.removeFromTop(16);

    for (const auto& section : dna->sections)
    {
        auto row = bounds.removeFromTop(52);
        Theme::drawMuted(g, row.removeFromLeft(80), juce::String(section.name).toUpperCase());
        Theme::drawMuted(g, row.removeFromLeft(70), copy::formatTime(section.start));
        Theme::drawBlocks(g, row.removeFromTop(16), section.energy);
        row.removeFromTop(6);
        Theme::drawMuted(g, row, section.name == "drop" && section.energy >= 0.7f ? "The drop works"
                                                                                 : "Energy  " + juce::String(section.energy, 2));
        bounds.removeFromTop(6);
    }

    if (state_.uiMode() == UiMode::Advanced)
    {
        bounds.removeFromTop(8);
        Theme::drawMuted(g, bounds.removeFromTop(14), "ADVANCED DNA");
        Theme::drawBody(g, bounds.removeFromTop(16), "novelty peak + energy label");
    }
}

} // namespace sonora
