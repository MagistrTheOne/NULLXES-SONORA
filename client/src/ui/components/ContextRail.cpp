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
    g.setColour(Theme::card());
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);

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
        bounds.removeFromTop(16);
    };

    const juce::String track = state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK";
    juce::String engine = "DSP  OFFLINE";
    if (state_.backendOnline())
    {
        if (state_.analysisState() == AnalysisState::Analyzing || state_.analysisState() == AnalysisState::Loading)
            engine = "DSP  WORKING";
        else
            engine = "DSP  READY";
    }

    drawBlock("TRACK", { track });
    drawBlock("SOURCE", { state_.channelLabel(), state_.sampleRateLabel() });
    drawBlock("ENGINE", { engine });
    drawBlock("PROFILE", state_.profileLines());
}

} // namespace sonora
