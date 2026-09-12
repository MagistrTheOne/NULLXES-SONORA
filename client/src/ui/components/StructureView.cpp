#include "ui/components/StructureView.h"

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
    if (dna == nullptr)
        return;

    auto bounds = getLocalBounds();
    auto listBox = bounds.removeFromLeft(juce::jmax(280, bounds.getWidth() * 46 / 100));
    bounds.removeFromLeft(12);
    auto mapBox = bounds;

    theme::fillCard(g, listBox);
    auto list = listBox.reduced(16, 14);
    theme::drawSectionLabel(g, list.removeFromTop(14), "ARRANGEMENT");
    list.removeFromTop(10);
    const int rowH = juce::jmax(48, juce::jmin(64, list.getHeight() / juce::jmax(1, (int) dna->sections.size())));
    for (const auto& section : dna->sections)
    {
        auto row = list.removeFromTop(rowH);
        auto label = row.removeFromLeft(72);
        Theme::drawMuted(g, label.removeFromTop(12), juce::String(section.name).toUpperCase());
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawText(
            juce::String(section.start, 1) + " - " + juce::String(section.end, 1),
            label,
            juce::Justification::centredLeft,
            true);
        row.removeFromLeft(12);
        Theme::drawBlocks(g, row.removeFromTop(16), section.energy);
        row.removeFromTop(6);
        Theme::drawMuted(g, row, "ENERGY  " + juce::String(section.energy, 2)
            + "    BASS  " + juce::String(section.bassEnergy, 2));
        list.removeFromTop(6);
    }

    theme::fillCard(g, mapBox);
    auto map = mapBox.reduced(16, 14);
    theme::drawSectionLabel(g, map.removeFromTop(14), "ENERGY PROFILE");
    map.removeFromTop(10);

    auto curveBox = map.removeFromTop(juce::jmax(56, map.getHeight() / 3));
    g.setColour(colors::border());
    g.fillRect(curveBox);
    if (!dna->energyCurve.empty())
    {
        const float slot = (float) curveBox.getWidth() / (float) dna->energyCurve.size();
        for (int i = 0; i < (int) dna->energyCurve.size(); ++i)
        {
            const float value = juce::jlimit(0.0f, 1.0f, dna->energyCurve[(size_t) i]);
            const int h = juce::jmax(1, juce::roundToInt((float) curveBox.getHeight() * value));
            g.setColour(colors::foreground());
            g.fillRect(curveBox.getX() + juce::roundToInt(slot * (float) i),
                       curveBox.getBottom() - h,
                       juce::jmax(1, juce::roundToInt(slot) - 1),
                       h);
        }
    }
    map.removeFromTop(16);
    Theme::drawMuted(g, map.removeFromTop(14), "MEAN  " + juce::String(dna->energyMean, 2)
        + "    PEAK  " + juce::String(dna->energyPeak, 2));
    map.removeFromTop(12);
    theme::drawSectionLabel(g, map.removeFromTop(14), "BY SECTION");
    map.removeFromTop(10);
    for (const auto& section : dna->sections)
    {
        auto row = map.removeFromTop(22);
        Theme::drawMuted(g, row.removeFromLeft(72), juce::String(section.name).toUpperCase());
        Theme::drawBlocks(g, row.reduced(0, 4), section.energy);
        map.removeFromTop(6);
    }
}

} // namespace sonora
