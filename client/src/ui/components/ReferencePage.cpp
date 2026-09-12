#include "ui/components/ReferencePage.h"

#include "backend/ClientLog.h"
#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

ReferencePage::ReferencePage(AppState& state) : state_(state)
{
    load_.setLabel("LOAD REFERENCE");
    load_.onClick = [this] { chooseReference(); };
    addAndMakeVisible(load_);
}

void ReferencePage::chooseReference()
{
    chooser_ = std::make_unique<juce::FileChooser>(
        "LOAD REFERENCE",
        juce::File(),
        "*.wav;*.mp3;*.flac");
    constexpr auto flags = juce::FileBrowserComponent::openMode
                           | juce::FileBrowserComponent::canSelectFiles;
    chooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
        const auto file = chooser.getResult();
        clientLog("Reference chooser " + file.getFullPathName());
        if (file.existsAsFile())
            state_.compareReference(file);
    });
}

void ReferencePage::paint(juce::Graphics& g)
{
    const bool ready = state_.analysisState() == AnalysisState::Complete && !state_.audioId().isEmpty();
    load_.setEnabled(ready && !state_.referenceBusy());

    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(24, 22);
    Theme::drawMuted(g, bounds.removeFromTop(14), "REFERENCE A / B");
    bounds.removeFromTop(10);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("Two files. One gap.", bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(8);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    ready ? juce::String(state_.loadedFilename()) + "   vs   a reference you pick"
                          : "Load a track first, then a reference.");
    bounds.removeFromTop(8);
    Theme::drawMuted(g, bounds.removeFromTop(14), "DSP delta. Not stems. Not a score.");
    bounds.removeFromTop(52);

    if (state_.referenceBusy())
    {
        Theme::drawBody(g, bounds.removeFromTop(20), "Comparing the two files...");
        return;
    }

    const auto* report = state_.reference() ? &*state_.reference() : nullptr;
    if (report == nullptr)
    {
        Theme::drawMuted(g, bounds.removeFromTop(16), "Load a finished track you trust. SONORA will measure the gap.");
        return;
    }

    Theme::drawMuted(g, bounds.removeFromTop(14), "A / B");
    Theme::drawBody(g, bounds.removeFromTop(18),
                    juce::String(report->targetFilename) + "    /    " + juce::String(report->referenceFilename));
    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "GAP");
    Theme::drawBody(g, bounds.removeFromTop(18),
                    "Loudness  " + juce::String(report->gap.loudnessLufs >= 0 ? "+" : "")
                        + juce::String(report->gap.loudnessLufs, 1) + " LUFS");
    Theme::drawBody(g, bounds.removeFromTop(18), "Low end  " + copy::signedPercent(report->gap.lowEnd));
    Theme::drawBody(g, bounds.removeFromTop(18), "Stereo  " + copy::signedPercent(report->gap.stereo));
    Theme::drawBody(g, bounds.removeFromTop(18), "Brightness  " + copy::signedPercent(report->gap.brightness));
    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "NOTES");
    for (const auto& note : report->notes)
        Theme::drawBody(g, bounds.removeFromTop(18), juce::String(note));
}

void ReferencePage::resized()
{
    auto bounds = getLocalBounds().reduced(24, 22);
    bounds.removeFromTop(86);
    load_.setBounds(bounds.removeFromTop(40).removeFromLeft(180));
}

} // namespace sonora
