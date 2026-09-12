#include "ui/components/StatusRail.h"

#include "ui/theme/Theme.h"

namespace sonora
{

StatusRail::StatusRail(AppState& state)
    : state_(state)
{
    setOpaque(true);
}

void StatusRail::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());
    g.setColour(colors::border());
    g.fillRect(0, 0, getWidth(), 1);

    auto bounds = getLocalBounds().reduced(20, 0);
    const int cell = bounds.getWidth() / 3;

    auto paintCell = [&](juce::Rectangle<int> area, const juce::String& k, const juce::String& v) {
        g.setColour(colors::mutedForeground());
        g.setFont(type::label(9.0f));
        g.drawText(k, area.removeFromLeft(80), juce::Justification::centredLeft, true);
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawText(v, area, juce::Justification::centredLeft, true);
    };

    juce::String compute = state_.backendOnline() ? "READY" : "OFFLINE";
    if (state_.analysisState() == AnalysisState::Analyzing || state_.analysisState() == AnalysisState::Loading)
        compute = "LISTENING";

    juce::String result = "---";
    if (state_.analysisState() == AnalysisState::Complete)
        result = state_.issueCountLabel() + "  /  " + state_.objectCountLabel();
    else if (state_.analysisState() == AnalysisState::Failed)
        result = state_.fault().reason.isEmpty() ? "FAILED" : state_.fault().reason;

    paintCell(bounds.removeFromLeft(cell), "COMPUTE", compute);
    paintCell(bounds.removeFromLeft(cell), "SONORA", "v0.3.1");
    paintCell(bounds, "RESULT", result);
}

} // namespace sonora
