#include "ui/components/ContextRail.h"

#include "ui/theme/Theme.h"

namespace sonora
{

ContextRail::ContextRail(AppState& state)
    : state_(state)
{
}

void ContextRail::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(18, 20);
    Theme::drawLabel(g, bounds.removeFromTop(12), "SESSION");
    bounds.removeFromTop(18);

    auto drawBlock = [&](const juce::String& title, const std::vector<juce::String>& lines) {
        Theme::drawLabel(g, bounds.removeFromTop(12), title);
        bounds.removeFromTop(8);
        for (const auto& line : lines)
        {
            Theme::drawBody(g, bounds.removeFromTop(18), line);
            bounds.removeFromTop(4);
        }
        bounds.removeFromTop(14);
    };

    const juce::String track = state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK";
    juce::String engine = "LOCAL DSP";
    if (state_.analysisState() == AnalysisState::Failed)
        engine = "DSP FAILED";
    else if (state_.isListening()
             || state_.analysisState() == AnalysisState::Analyzing
             || state_.analysisState() == AnalysisState::Loading)
        engine = "LISTENING";
    else if (state_.analysisState() == AnalysisState::Complete)
        engine = "DSP READY";

    drawBlock("TRACK", { track });
    drawBlock("DURATION", { state_.durationLabel() });
    drawBlock("FORMAT", { state_.channelLabel(), state_.sampleRateLabel(), state_.bitDepthLabel() });
    drawBlock("ENGINE", {
        state_.dawHost()
            ? (state_.trackUnderstood() ? "UNDERSTOOD"
               : state_.hostPlaying() ? "LIVE PLAY"
                                      : "LIVE IDLE")
            : engine
    });
    if (state_.dawHost())
        drawBlock("NOW", { state_.barLabel(), state_.nowSection(), state_.bpmLabel() + " BPM" });

    std::vector<juce::String> identity;
    for (const auto& axis : state_.sonicIdentity())
        identity.push_back(axis.name + "  " + juce::String(juce::roundToInt(axis.value * 100.0f)) + "%");
    if (identity.empty())
        identity.emplace_back("Waiting");
    drawBlock("IDENTITY", identity);
}

} // namespace sonora
