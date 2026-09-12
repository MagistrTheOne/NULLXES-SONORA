#include "ui/components/ArrangementStrip.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

ArrangementStrip::ArrangementStrip(AppState& state) : state_(state) {}

void ArrangementStrip::mouseUp(const juce::MouseEvent& event)
{
    if (getLocalBounds().contains(event.getPosition()))
        state_.focusArrangement();
}

void ArrangementStrip::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    auto header = bounds.removeFromTop(16);
    Theme::drawMuted(g, header.removeFromLeft(header.getWidth() - 90), "ARRANGEMENT");
    Theme::drawMuted(g, header, "VIEW DETAILS");
    bounds.removeFromTop(10);

    const auto* dna = state_.dna();
    if (dna == nullptr || dna->sections.empty() || !state_.analysis())
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Load a track to see form");
        return;
    }

    const float duration = juce::jmax(0.1f, state_.analysis()->durationSec);
    auto lane = bounds.removeFromTop(28);
    int x = lane.getX();
    for (const auto& section : dna->sections)
    {
        const float span = juce::jmax(0.0f, section.end - section.start) / duration;
        const int w = juce::jmax(4, juce::roundToInt((float) lane.getWidth() * span));
        auto slice = juce::Rectangle<int>(x, lane.getY(), juce::jmin(w, lane.getRight() - x), lane.getHeight());
        const bool drop = section.name == "drop";
        g.setColour(drop ? colors::destructive().withAlpha(0.85f)
                         : colors::foreground().withAlpha(0.18f + section.energy * 0.45f));
        g.fillRect(slice);
        x += w;
    }
    bounds.removeFromTop(12);

    const int n = (int) dna->sections.size();
    const int cell = n > 0 ? bounds.getWidth() / n : bounds.getWidth();
    for (const auto& section : dna->sections)
    {
        auto cellBounds = bounds.removeFromLeft(cell);
        Theme::drawMuted(g, cellBounds.removeFromTop(12), juce::String(section.name).toUpperCase());
        Theme::drawMuted(g, cellBounds, copy::formatTime(section.start));
    }
}

} // namespace sonora
