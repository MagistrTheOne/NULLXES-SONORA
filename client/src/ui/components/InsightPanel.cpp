#include "ui/components/InsightPanel.h"

#include "ui/theme/Theme.h"

namespace sonora
{

InsightPanel::InsightPanel(AppState& state)
    : state_(state)
{
    generate_.setLabel("GENERATE ENGINEERING REPORT");
    generate_.onClick = [this] { state_.requestEngineeringReport(); };
    createEq_.setLabel("CREATE EQ PROFILE");
    createEq_.onClick = [this] { state_.createEqProfile(); };
    addAndMakeVisible(generate_);
    addAndMakeVisible(createEq_);
}

void InsightPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(16, 14);
    theme::drawSectionLabel(g, bounds.removeFromTop(16), "SONORA INSIGHT");
    bounds.removeFromTop(10);
    bounds.removeFromBottom(76);

    auto drawField = [&g, &bounds](const juce::String& k, const juce::String& v) {
        theme::drawSectionLabel(g, bounds.removeFromTop(12), k);
        bounds.removeFromTop(4);
        g.setColour(colors::foreground());
        g.setFont(type::body(13.0f));
        g.drawText(v, bounds.removeFromTop(18), juce::Justification::centredLeft, true);
        bounds.removeFromTop(10);
    };

    if (state_.analysisState() != AnalysisState::Complete)
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawMultiLineText(
            "Load a track. SONORA will detect a problem, propose an operation, then create an object.",
            bounds.getX(),
            bounds.getY() + 14,
            bounds.getWidth());
        return;
    }

    juce::String issue = "No issues yet";
    juce::String confidence = "---";
    juce::String action = "Generate engineering report";
    juce::String target = "120Hz";

    if (!state_.insights().empty())
    {
        const auto& insight = state_.insights().front();
        issue = insight.reason.empty() ? insight.issue : insight.reason;
        if (issue.isEmpty())
            issue = insight.action;
        confidence = juce::String(juce::roundToInt(insight.confidence * 100.0f)) + "%";
        action = insight.action.empty() ? "Create EQ profile" : juce::String(insight.action);
        if (insight.frequencyHz.has_value())
            target = juce::String(juce::roundToInt(*insight.frequencyHz)) + "Hz";
    }
    else if (!state_.issues().empty())
    {
        const auto& first = state_.issues().front();
        issue = first.detail.empty() ? first.type : first.detail;
        confidence = juce::String(juce::roundToInt(first.severity * 100.0f)) + "%";
        action = "Create EQ profile";
    }

    drawField("ISSUE", issue);
    drawField("CONFIDENCE", confidence);
    drawField("ACTION", action);
    drawField("TARGET", target);

    if (state_.eqProfile())
    {
        bounds.removeFromTop(4);
        theme::drawSectionLabel(g, bounds.removeFromTop(12), "EQ OBJECT");
        bounds.removeFromTop(4);
        g.setColour(colors::foreground());
        g.setFont(type::body(13.0f));
        g.drawText(
            juce::String(state_.eqProfile()->operation) + "  "
                + juce::String(juce::roundToInt(state_.eqProfile()->frequencyHz)) + "Hz  "
                + juce::String(state_.eqProfile()->target),
            bounds.removeFromTop(18),
            juce::Justification::centredLeft,
            true);
    }
}

void InsightPanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    createEq_.setBounds(bounds.removeFromBottom(32));
    bounds.removeFromBottom(8);
    generate_.setBounds(bounds.removeFromBottom(32));
}

} // namespace sonora
