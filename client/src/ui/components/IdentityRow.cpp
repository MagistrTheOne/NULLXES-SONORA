#include "ui/components/IdentityRow.h"

#include "ui/theme/Theme.h"

namespace sonora
{

IdentityRow::IdentityRow(AppState& state) : state_(state) {}

void IdentityRow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    const int gap = 10;
    const int w = (bounds.getWidth() - gap * 4) / 5;
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
    cell("LOUDNESS", ready ? juce::String(state_.analysis()->loudnessLufsApprox, 1) : "---", "LUFS");

    auto energyBox = bounds.removeFromLeft(w);
    bounds.removeFromLeft(gap);
    theme::fillCard(g, energyBox);
    auto energy = energyBox.reduced(12, 10);
    Theme::drawMuted(g, energy.removeFromTop(12), "ENERGY");
    energy.removeFromTop(8);
    const float energyValue = dna != nullptr ? dna->energyMean : 0.0f;
    Theme::drawBlocks(g, energy.removeFromTop(14), energyValue);
    energy.removeFromTop(8);
    Theme::drawMuted(g, energy, ready ? juce::String(juce::roundToInt(energyValue * 100.0f)) + "%" : "---");

    auto genreBox = bounds;
    theme::fillCard(g, genreBox);
    auto genre = genreBox.reduced(12, 10);
    Theme::drawMuted(g, genre.removeFromTop(12), "GENRE");
    genre.removeFromTop(6);
    const auto styles = state_.profileLines();
    g.setColour(colors::foreground());
    g.setFont(type::body(14.0f));
    g.drawText(ready ? state_.styleLabel() : "---", genre.removeFromTop(18), juce::Justification::centredLeft, true);
    if (styles.size() > 1)
        Theme::drawMuted(g, genre.removeFromTop(16), styles[1]);
    genre.removeFromTop(4);
    juce::String mood;
    for (const auto& tag : state_.moodLabels())
    {
        if (mood.isNotEmpty())
            mood << "  ";
        mood << tag;
    }
    Theme::drawMuted(g, genre, mood);
}

} // namespace sonora
