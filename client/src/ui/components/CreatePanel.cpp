#include "ui/components/CreatePanel.h"

#include "ui/theme/Theme.h"

namespace sonora
{

CreatePanel::CreatePanel(AppState& state)
    : state_(state)
{
    chords_.setLabel("CHORD");
    bassline_.setLabel("BASS");
    arrangement_.setLabel("ARRANGEMENT");
    midi_.setLabel("MIDI");
    chords_.onClick = [this] { state_.requestHarmony(); };
    bassline_.onClick = [this] { state_.requestBass(); };
    arrangement_.onClick = [this] { state_.requestArrangement(); };
    midi_.onClick = [this] { state_.requestPad(); };
    for (auto* button : { &chords_, &bassline_, &arrangement_, &midi_ })
        addAndMakeVisible(*button);
}

void CreatePanel::paint(juce::Graphics& g)
{
    const bool ready = state_.analysisState() == AnalysisState::Complete;
    chords_.setEnabled(ready);
    bassline_.setEnabled(ready);
    arrangement_.setEnabled(ready && state_.dna() != nullptr);
    midi_.setEnabled(ready);

    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(16, 14);
    Theme::drawLabel(g, bounds.removeFromTop(12), "CREATE");
    bounds.removeFromTop(8);
    Theme::drawMuted(g, bounds.removeFromTop(14), "WRITE AN OBJECT");
    bounds.removeFromTop(100);
    Theme::drawMuted(g, bounds.removeFromTop(14), "CONTEXT");
    bounds.removeFromTop(8);

    Theme::drawBody(g, bounds.removeFromTop(16), ready ? state_.bpmLabel() + " BPM" : "--- BPM");
    Theme::drawBody(g, bounds.removeFromTop(16), ready ? state_.keyLabel() : "---");
    Theme::drawBody(g, bounds.removeFromTop(16), ready ? state_.styleLabel() : "---");

    if (state_.harmony())
    {
        bounds.removeFromTop(12);
        Theme::drawMuted(g, bounds.removeFromTop(14), "MIDI OBJECT");
        Theme::drawBody(g, bounds.removeFromTop(16),
                        juce::String(state_.harmony()->key) + " / " + juce::String(state_.harmony()->bars) + " bars");
    }
}

void CreatePanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    bounds.removeFromTop(34);
    auto grid = bounds.removeFromTop(92);
    const int gap = 8;
    const int w = (grid.getWidth() - gap) / 2;
    const int h = (grid.getHeight() - gap) / 2;
    chords_.setBounds(grid.getX(), grid.getY(), w, h);
    bassline_.setBounds(grid.getX() + w + gap, grid.getY(), w, h);
    arrangement_.setBounds(grid.getX(), grid.getY() + h + gap, w, h);
    midi_.setBounds(grid.getX() + w + gap, grid.getY() + h + gap, w, h);
}

} // namespace sonora
