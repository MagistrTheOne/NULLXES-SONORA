#include "ui/components/StatusRail.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
juce::String computeLabel(const AppState& state)
{
    if (!state.backendOnline())
        return "OFFLINE";
    switch (state.analysisState())
    {
        case AnalysisState::Loading:
            return "LOADING";
        case AnalysisState::Analyzing:
            return "ANALYZING";
        case AnalysisState::Complete:
            return "COMPLETE";
        case AnalysisState::Failed:
            return "FAILED";
        case AnalysisState::Empty:
        default:
            return "READY";
    }
}

juce::String resultLabel(const AppState& state)
{
    if (state.analysisState() == AnalysisState::Failed)
        return state.lastError().isEmpty() ? "FAILED" : state.lastError();
    if (state.analysisState() == AnalysisState::Complete)
        return state.issueCountLabel();
    if (state.analysisState() == AnalysisState::Analyzing || state.analysisState() == AnalysisState::Loading)
        return "PENDING";
    return "---";
}
} // namespace

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
        g.drawFittedText(k, area.removeFromLeft(72), juce::Justification::centredLeft, 1);
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawFittedText(v, area, juce::Justification::centredLeft, 1);
    };

    paintCell(bounds.removeFromLeft(cell), "COMPUTE", computeLabel(state_));
    paintCell(bounds.removeFromLeft(cell), "USAGE", state_.backendOnline() ? "LOCAL" : "---");
    paintCell(bounds, "RESULT", resultLabel(state_));
}

} // namespace sonora
