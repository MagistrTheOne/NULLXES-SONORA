#include "ui/components/LabOverlay.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

LabOverlay::LabOverlay(AppState& state)
    : state_(state)
{
    for (auto* button : { &option0_, &option1_, &option2_, &option3_ })
        addAndMakeVisible(*button);
}

juce::Rectangle<int> LabOverlay::cardBounds() const
{
    return getLocalBounds().withSizeKeepingCentre(juce::jmin(560, getWidth() - 80), juce::jmin(420, getHeight() - 80));
}

void LabOverlay::syncOptions()
{
    const auto options = state_.assistOptions();
    ActionButton* buttons[] = { &option0_, &option1_, &option2_, &option3_ };
    const bool ready = state_.analysisState() == AnalysisState::Complete;
    for (int i = 0; i < 4; ++i)
    {
        if (i < (int) options.size())
        {
            const auto id = juce::String(options[(size_t) i].id);
            buttons[i]->setLabel(juce::String(options[(size_t) i].label));
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

void LabOverlay::paint(juce::Graphics& g)
{
    syncOptions();
    g.setColour(colors::background().withAlpha(0.72f));
    g.fillRect(getLocalBounds());

    const auto card = cardBounds();
    theme::fillCard(g, card);
    auto bounds = card.reduced(28, 24);
    Theme::drawMuted(g, bounds.removeFromTop(14), "CTRL+L");
    bounds.removeFromTop(6);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("SONORA LAB", bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(10);
    Theme::drawBody(g, bounds.removeFromTop(20), "What do you want to change?");
    bounds.removeFromTop(8);

    if (state_.analysisState() == AnalysisState::Complete)
    {
        const auto finding = state_.assistFinding();
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(13.0f));
        g.drawMultiLineText(finding.headline, bounds.getX(), bounds.getY() + 14, bounds.getWidth());
    }
    else
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Load a track first.");
    }
}

void LabOverlay::resized()
{
    syncOptions();
    auto bounds = cardBounds().reduced(28, 24);
    bounds.removeFromTop(118);
    ActionButton* buttons[] = { &option0_, &option1_, &option2_, &option3_ };
    for (auto* button : buttons)
    {
        if (!button->isVisible())
            continue;
        button->setBounds(bounds.removeFromTop(40));
        bounds.removeFromTop(8);
    }
}

void LabOverlay::mouseUp(const juce::MouseEvent& event)
{
    if (!cardBounds().contains(event.getPosition()))
        state_.closeLab();
}

} // namespace sonora
