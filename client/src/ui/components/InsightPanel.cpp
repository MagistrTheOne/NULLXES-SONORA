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
        g.drawFittedText(v, bounds.removeFromTop(18), juce::Justification::centredLeft, 1);
        bounds.removeFromTop(10);
    };

    if (state_.analysisState() != AnalysisState::Complete)
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawFittedText(
            "Load a track. SONORA will detect a problem, propose an operation, then create an object.",
            bounds.removeFromTop(56),
            juce::Justification::topLeft,
            3);
        return;
    }

    if (state_.insights().empty())
    {
        juce::String detected = state_.issues().empty() ? "No issues yet. Generate an engineering report."
                                                       : juce::String(state_.issues().front().detail.empty()
                                                                          ? state_.issues().front().type
                                                                          : state_.issues().front().detail);
        drawField("DETECTED", detected);
        drawField("CONFIDENCE", state_.issues().empty() ? "—"
                                                       : juce::String(juce::roundToInt(state_.issues().front().severity * 100.0f)) + "%");
        drawField("SUGGESTED OPERATION", "Dynamic EQ");
        drawField("TARGET", "120Hz");
        return;
    }

    const auto& insight = state_.insights().front();
    drawField("DETECTED", insight.reason.empty() ? insight.issue : insight.reason);
    drawField("CONFIDENCE", juce::String(juce::roundToInt(insight.confidence * 100.0f)) + "%");
    drawField("SUGGESTED OPERATION", insight.operation.empty() ? insight.action : insight.operation);
    drawField("TARGET",
              insight.frequencyHz.has_value() ? juce::String(juce::roundToInt(*insight.frequencyHz)) + "Hz"
                                              : (insight.issue.empty() ? insight.action : juce::String(insight.issue)));

    if (state_.eqProfile())
    {
        bounds.removeFromTop(4);
        theme::drawSectionLabel(g, bounds.removeFromTop(12), "EQ OBJECT");
        bounds.removeFromTop(4);
        g.setColour(colors::foreground());
        g.setFont(type::body(13.0f));
        g.drawFittedText(
            juce::String(state_.eqProfile()->operation) + "  ·  "
                + juce::String(juce::roundToInt(state_.eqProfile()->frequencyHz)) + "Hz  ·  "
                + juce::String(state_.eqProfile()->target),
            bounds.removeFromTop(18),
            juce::Justification::centredLeft,
            1);
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
