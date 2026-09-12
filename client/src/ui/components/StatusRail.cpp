#include "ui/components/StatusRail.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
juce::String computeLabel(const AppState& state)
{
    return state.backendOnline() ? "BACKEND ONLINE" : "BACKEND OFFLINE";
}

juce::String usageLabel(const AppState& state)
{
    switch (state.analysisState())
    {
        case AnalysisState::Loading:
        case AnalysisState::Analyzing:
            return "ANALYZING";
        case AnalysisState::Failed:
            return "FAILED";
        case AnalysisState::Complete:
            return "COMPLETE";
        case AnalysisState::Empty:
        default:
            return state.backendOnline() ? "IDLE" : "OFFLINE";
    }
}

juce::String resultLabel(const AppState& state)
{
    switch (state.analysisState())
    {
        case AnalysisState::Loading:
        case AnalysisState::Analyzing:
            return "WAITING";
        case AnalysisState::Complete:
            return state.issueCountLabel();
        case AnalysisState::Failed:
            return state.fault().reason.isEmpty() ? "FAILED" : state.fault().reason;
        case AnalysisState::Empty:
        default:
            return "---";
    }
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
        g.drawText(k, area.removeFromLeft(72), juce::Justification::centredLeft, true);
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawText(v, area, juce::Justification::centredLeft, true);
    };

    paintCell(bounds.removeFromLeft(cell), "COMPUTE", computeLabel(state_));
    paintCell(bounds.removeFromLeft(cell), "USAGE", usageLabel(state_));
    paintCell(bounds, "RESULT", resultLabel(state_));
}

} // namespace sonora
