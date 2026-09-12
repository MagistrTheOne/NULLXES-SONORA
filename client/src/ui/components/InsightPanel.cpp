#include "ui/components/InsightPanel.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

InsightPanel::InsightPanel(AppState& state)
    : state_(state)
{
    for (auto* button : { &option0_, &option1_, &option2_, &option3_ })
        addAndMakeVisible(*button);
}

void InsightPanel::syncOptions()
{
    const auto options = state_.assistOptions();
    ActionButton* buttons[] = { &option0_, &option1_, &option2_, &option3_ };
    const bool ready = state_.analysisState() == AnalysisState::Complete;
    for (int i = 0; i < 4; ++i)
    {
        if (i < (int) options.size())
        {
            const auto id = juce::String(options[(size_t) i].id);
            auto label = juce::String(options[(size_t) i].label).toUpperCase();
            buttons[i]->setLabel(label);
            buttons[i]->setVisible(true);
            buttons[i]->setEnabled(ready);
            buttons[i]->onClick = [this, id] { state_.applyAssistOption(id); };
        }
        else
        {
            buttons[i]->setVisible(false);
            buttons[i]->setEnabled(false);
            buttons[i]->onClick = nullptr;
        }
    }
}

void InsightPanel::paint(juce::Graphics& g)
{
    syncOptions();
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 14);
    const auto title = state_.assistArmed() ? "SONORA ASSIST" : "SONORA INSIGHT";
    theme::drawSectionLabel(g, bounds.removeFromTop(16), title);
    bounds.removeFromTop(8);

    const int visible = (int) juce::jmin((size_t) 4, state_.assistOptions().size());
    bounds.removeFromBottom(visible * 36);

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

    if (state_.dropPlan())
    {
        bounds.removeFromTop(8);
        Theme::drawMuted(g, bounds.removeFromTop(12), "DROP OBJECT");
        Theme::drawBody(g, bounds.removeFromTop(16),
                        juce::String(state_.dropPlan()->sectionName) + "  "
                            + copy::formatTime(state_.dropPlan()->start) + " – "
                            + copy::formatTime(state_.dropPlan()->end));
    }
}

void InsightPanel::resized()
{
    syncOptions();
    auto bounds = getLocalBounds().reduced(16, 14);
    ActionButton* buttons[] = { &option3_, &option2_, &option1_, &option0_ };
    for (auto* button : buttons)
    {
        if (!button->isVisible())
            continue;
        button->setBounds(bounds.removeFromBottom(28));
        bounds.removeFromBottom(8);
    }
}

} // namespace sonora
