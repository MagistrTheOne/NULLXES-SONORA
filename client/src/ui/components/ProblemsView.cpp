#include "ui/components/ProblemsView.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

ProblemsView::ProblemsView(AppState& state) : state_(state) {}

void ProblemsView::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(14), "SONORA FOUND");
    bounds.removeFromTop(8);

    const auto found = state_.sonoraFound();
    if (state_.analysisState() != AnalysisState::Complete)
    {
        Theme::drawMuted(g, bounds.removeFromTop(18),
                         state_.dawHost() ? "Listening to the session" : "Waiting for a track");
        return;
    }
    for (const auto& line : found)
        Theme::drawBody(g, bounds.removeFromTop(20), line);
    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "PROBLEMS");
    bounds.removeFromTop(6);

    const auto& issues = state_.issues();
    if (issues.empty())
    {
        Theme::drawBody(g, bounds.removeFromTop(18), "No urgent collisions");
        Theme::drawMuted(g, bounds.removeFromTop(16), "The foundation is solid.");
        return;
    }

    const int rowH = juce::jmax(28, bounds.getHeight() / (int) juce::jmax((size_t) 1, issues.size()));
    for (const auto& issue : issues)
    {
        auto row = bounds.removeFromTop(rowH);
        auto mark = row.removeFromLeft(8).withSizeKeepingCentre(6, 6);
        g.setColour(theme::severityColour(issue.severity));
        g.fillRect(mark);
        row.removeFromLeft(10);
        auto meter = row.removeFromRight(72);
        Theme::drawBody(g, row.removeFromTop(16), copy::issuePhrase(issue));
        Theme::drawMuted(g, row, juce::String(issue.detail));
        g.setColour(colors::border());
        auto track = meter.withSizeKeepingCentre(meter.getWidth(), 4);
        g.fillRect(track);
        g.setColour(theme::severityColour(issue.severity));
        g.fillRect(track.withWidth(juce::jmax(2, juce::roundToInt((float) track.getWidth() * issue.severity))));
    }
}

} // namespace sonora
