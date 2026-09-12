#include "ui/components/InsightPanel.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

InsightPanel::InsightPanel(AppState& state)
    : state_(state)
{
    generate_.setLabel("STRENGTHEN DROP");
    createEq_.setLabel("FIX VOCAL SPACE");
    generate_.onClick = [this] { state_.requestEngineeringReport(); };
    createEq_.onClick = [this] { state_.createEqProfile(); };
    addAndMakeVisible(generate_);
    addAndMakeVisible(createEq_);
}

void InsightPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 14);
    const auto title = state_.assistArmed() ? "SONORA ASSIST" : "SONORA INSIGHT";
    theme::drawSectionLabel(g, bounds.removeFromTop(16), title);
    bounds.removeFromTop(8);
    bounds.removeFromBottom(76);

    if (state_.analysisState() != AnalysisState::Complete)
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawMultiLineText(
            "Load a track. SONORA will listen, then we can work.",
            bounds.getX(),
            bounds.getY() + 14,
            bounds.getWidth());
        return;
    }

    const auto finding = state_.assistFinding();
    Theme::drawMuted(g, bounds.removeFromTop(12), "I FOUND");
    bounds.removeFromTop(4);
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawMultiLineText(finding.headline, bounds.getX(), bounds.getY() + 14, bounds.getWidth());
    bounds.removeFromTop(36);
    g.setColour(colors::mutedForeground());
    g.setFont(type::body(12.0f));
    g.drawMultiLineText(finding.detail, bounds.getX(), bounds.getY() + 12, bounds.getWidth());
    bounds.removeFromTop(48);

    const auto delta = state_.mixDelta();
    if (delta.valid)
    {
        Theme::drawMuted(g, bounds.removeFromTop(12), "WHAT CHANGED");
        bounds.removeFromTop(6);
        Theme::drawBody(g, bounds.removeFromTop(16), "Bass clarity  " + copy::signedPercent(delta.lowEnd));
        Theme::drawBody(g, bounds.removeFromTop(16), "Stereo  " + copy::signedPercent(delta.stereo));
        Theme::drawBody(g, bounds.removeFromTop(16),
                        "Loudness  " + juce::String(delta.loudness >= 0 ? "+" : "") + juce::String(delta.loudness, 1) + " LUFS");
    }

    if (state_.eqProfile())
    {
        bounds.removeFromTop(8);
        Theme::drawMuted(g, bounds.removeFromTop(12), "EQ OBJECT");
        Theme::drawBody(g, bounds.removeFromTop(16),
                        juce::String(juce::roundToInt(state_.eqProfile()->frequencyHz)) + "Hz  "
                            + juce::String(state_.eqProfile()->gainDb, 1) + "dB");
    }

    generate_.setEnabled(false);
    createEq_.setEnabled(true);
}

void InsightPanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    createEq_.setBounds(bounds.removeFromBottom(32));
    bounds.removeFromBottom(8);
    generate_.setBounds(bounds.removeFromBottom(32));
}

} // namespace sonora
