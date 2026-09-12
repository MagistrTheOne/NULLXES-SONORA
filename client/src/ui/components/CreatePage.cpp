#include "ui/components/CreatePage.h"

#include "ui/theme/Theme.h"

namespace sonora
{

CreatePage::CreatePage(AppState& state) : state_(state)
{
    chords_.setLabel("CHORDS");
    bass_.setLabel("BASS");
    pad_.setLabel("PAD");
    arrangement_.setLabel("ARRANGEMENT");
    vocal_.setLabel("VOCAL SPACE");
    chords_.onClick = [this] { state_.requestHarmony(); };
    arrangement_.onClick = [this] { state_.focusArrangement(); };
    vocal_.onClick = [this] { state_.createEqProfile(); };
    for (auto* button : { &chords_, &bass_, &pad_, &arrangement_, &vocal_ })
        addAndMakeVisible(*button);
}

void CreatePage::paint(juce::Graphics& g)
{
    const bool ready = state_.analysisState() == AnalysisState::Complete;
    chords_.setEnabled(ready);
    bass_.setEnabled(false);
    pad_.setEnabled(false);
    arrangement_.setEnabled(ready && state_.dna() != nullptr);
    vocal_.setEnabled(ready);

    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(24, 22);
    Theme::drawMuted(g, bounds.removeFromTop(14), "CREATE WITH SONORA");
    bounds.removeFromTop(10);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("Your track", bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(8);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    ready ? state_.bpmLabel() + " BPM    " + state_.keyLabel() + "    " + state_.styleLabel()
                          : "Load a track first");
    bounds.removeFromTop(8);
    Theme::drawMuted(g, bounds.removeFromTop(14), "SONORA does not chat. It writes objects.");
    bounds.removeFromTop(120);

    if (state_.harmony())
    {
        Theme::drawMuted(g, bounds.removeFromTop(14), "MIDI CLIP");
        bounds.removeFromTop(4);
        Theme::drawBody(g, bounds.removeFromTop(18),
                        juce::String(state_.harmony()->key) + "  /  " + juce::String(state_.harmony()->bars) + " bars");
        juce::String line;
        for (const auto& chord : state_.harmony()->chords)
        {
            if (line.isNotEmpty())
                line << "   ";
            line << juce::String(chord);
        }
        Theme::drawBody(g, bounds.removeFromTop(20), line);
    }
}

void CreatePage::resized()
{
    auto bounds = getLocalBounds().reduced(24, 22);
    bounds.removeFromTop(108);
    auto grid = bounds.removeFromTop(96);
    const int gap = 8;
    const int w = (grid.getWidth() - gap * 2) / 3;
    chords_.setBounds(grid.removeFromLeft(w));
    grid.removeFromLeft(gap);
    bass_.setBounds(grid.removeFromLeft(w));
    grid.removeFromLeft(gap);
    pad_.setBounds(grid);
    bounds.removeFromTop(10);
    auto row = bounds.removeFromTop(40);
    arrangement_.setBounds(row.removeFromLeft((row.getWidth() - gap) / 2));
    row.removeFromLeft(gap);
    vocal_.setBounds(row);
}

} // namespace sonora
