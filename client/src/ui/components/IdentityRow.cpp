#include "ui/components/IdentityRow.h"

#include "ui/theme/Theme.h"

namespace sonora
{

IdentityRow::IdentityRow(AppState& state) : state_(state) {}

void IdentityRow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const int gap = 10;
    const int w = (bounds.getWidth() - gap * 3) / 4;
    const auto* dna = state_.dna();
    const bool ready = state_.analysisState() == AnalysisState::Complete && state_.analysis();

    auto cell = [&](const juce::String& label, const juce::String& value, const juce::String& hint) {
        auto box = bounds.removeFromLeft(w);
        bounds.removeFromLeft(gap);
        theme::fillCard(g, box);
        auto inner = box.reduced(12, 10);
        Theme::drawMuted(g, inner.removeFromTop(12), label);
        inner.removeFromTop(6);
        g.setColour(colors::foreground());
        g.setFont(type::display(20.0f));
        g.drawText(value, inner.removeFromTop(24), juce::Justification::centredLeft, true);
        Theme::drawMuted(g, inner, hint);
    };

    cell("TEMPO", ready ? state_.bpmLabel() : "---", "BPM");
    cell("KEY", ready ? state_.keyLabel() : "---",
         ready && state_.analysis()->key.key.has_value()
             ? juce::String(juce::roundToInt(state_.analysis()->key.confidence * 100.0f)) + "% confidence"
             : "unresolved");

    auto energyBox = bounds.removeFromLeft(w);
    bounds.removeFromLeft(gap);
    theme::fillCard(g, energyBox);
    auto energy = energyBox.reduced(12, 10);
    Theme::drawMuted(g, energy.removeFromTop(12), "ENERGY");
    energy.removeFromTop(4);
    g.setColour(colors::foreground());
    g.setFont(type::display(20.0f));
    g.drawText(ready ? state_.energyLabel() : "---", energy.removeFromTop(24), juce::Justification::centredLeft, true);
    Theme::drawBlocks(g, energy.removeFromTop(12), dna != nullptr ? state_.energyNow() : 0.0f);

    auto styleBox = bounds;
    theme::fillCard(g, styleBox);
    auto style = styleBox.reduced(12, 10);
    Theme::drawMuted(g, style.removeFromTop(12), "STYLE");
    style.removeFromTop(6);
    g.setColour(colors::foreground());
    g.setFont(type::body(14.0f));
    g.drawText(ready ? state_.styleLabel() : "---", style.removeFromTop(20), juce::Justification::centredLeft, true);
    Theme::drawMuted(g, style, "From the fingerprint, not a genre tag");
}

} // namespace sonora
