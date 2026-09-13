#include "ui/components/CreatePage.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
void drawClip(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& title, const models::MidiClip& clip)
{
    Theme::drawMuted(g, bounds.removeFromTop(14), title);
    bounds.removeFromTop(4);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    juce::String(clip.key) + "  /  " + juce::String(clip.bars) + " bars");
    juce::String line;
    const auto& tokens = !clip.notes.empty() ? clip.notes : clip.chords;
    for (const auto& token : tokens)
    {
        if (line.isNotEmpty())
            line << "   ";
        line << juce::String(token);
    }
    if (line.isNotEmpty())
        Theme::drawBody(g, bounds.removeFromTop(18), line);
    juce::String pattern;
    for (const auto& cell : clip.pattern)
    {
        if (pattern.isNotEmpty())
            pattern << "  ";
        pattern << juce::String(cell);
    }
    if (pattern.isNotEmpty())
        Theme::drawMuted(g, bounds.removeFromTop(14), pattern);
    bounds.removeFromTop(8);
}
} // namespace

CreatePage::CreatePage(AppState& state) : state_(state)
{
    chords_.setLabel("CHORD");
    bass_.setLabel("BASS");
    arrangement_.setLabel("ARRANGEMENT");
    midi_.setLabel("MIDI");
    chords_.onClick = [this] { state_.requestHarmony(); };
    bass_.onClick = [this] { state_.requestBass(); };
    arrangement_.onClick = [this] { state_.requestArrangement(); };
    midi_.onClick = [this] { exportMidi(); };
    for (auto* button : { &chords_, &bass_, &arrangement_, &midi_ })
        addAndMakeVisible(*button);
}

void CreatePage::exportMidi()
{
    chooser_ = std::make_unique<juce::FileChooser>(
        "EXPORT MIDI",
        juce::File(),
        "*.mid");
    constexpr auto chooserFlags = juce::FileBrowserComponent::saveMode
                           | juce::FileBrowserComponent::canSelectFiles
                           | juce::FileBrowserComponent::warnAboutOverwriting;
    chooser_->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser) {
        auto file = chooser.getResult();
        if (file.getFileName().isEmpty())
            return;
        if (!file.hasFileExtension(".mid"))
            file = file.withFileExtension(".mid");
        juce::String error;
        if (!state_.writeMidiFile(file, error))
            juce::ignoreUnused(error);
    });
}

void CreatePage::paint(juce::Graphics& g)
{
    const bool ready = state_.analysisState() == AnalysisState::Complete;
    chords_.setEnabled(ready);
    bass_.setEnabled(ready);
    arrangement_.setEnabled(ready && state_.dna() != nullptr);
    midi_.setEnabled(ready && (state_.harmony() || state_.bassClip() || state_.padClip()));

    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(24, 22);
    Theme::drawMuted(g, bounds.removeFromTop(14), "CREATE OBJECT");
    bounds.removeFromTop(10);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("Write the next part", bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(8);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    ready ? state_.bpmLabel() + " BPM    " + state_.keyLabel() + "    " + state_.styleLabel()
                          : "Load a track first");
    bounds.removeFromTop(8);
    Theme::drawMuted(g, bounds.removeFromTop(14), "SONORA writes objects. SONI talks.");
    bounds.removeFromTop(120);

    if (state_.harmony())
    {
        Theme::drawMuted(g, bounds.removeFromTop(14), "CHORD");
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
        Theme::drawBody(g, bounds.removeFromTop(18), line);
        bounds.removeFromTop(8);
    }
    if (state_.bassClip())
        drawClip(g, bounds, "BASS", *state_.bassClip());
    if (state_.padClip())
        drawClip(g, bounds, "MIDI LAYER", *state_.padClip());
    if (state_.dropPlan())
    {
        const auto& plan = *state_.dropPlan();
        Theme::drawMuted(g, bounds.removeFromTop(14), "DROP");
        bounds.removeFromTop(4);
        Theme::drawBody(g, bounds.removeFromTop(18),
                        juce::String(plan.sectionName) + "  "
                            + copy::formatTime(plan.start) + " – " + copy::formatTime(plan.end));
        for (const auto& action : plan.actions)
            Theme::drawBody(g, bounds.removeFromTop(18), juce::String(action));
    }
}

void CreatePage::resized()
{
    auto bounds = getLocalBounds().reduced(24, 22);
    bounds.removeFromTop(108);
    auto grid = bounds.removeFromTop(44);
    const int gap = 8;
    const int w = (grid.getWidth() - gap * 3) / 4;
    chords_.setBounds(grid.removeFromLeft(w));
    grid.removeFromLeft(gap);
    bass_.setBounds(grid.removeFromLeft(w));
    grid.removeFromLeft(gap);
    arrangement_.setBounds(grid.removeFromLeft(w));
    grid.removeFromLeft(gap);
    midi_.setBounds(grid);
}

} // namespace sonora
