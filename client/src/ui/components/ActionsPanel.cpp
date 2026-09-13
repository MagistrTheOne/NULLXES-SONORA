#include "ui/components/ActionsPanel.h"

#include "ui/theme/Theme.h"

namespace sonora
{

ActionsPanel::ActionsPanel(AppState& state)
    : state_(state)
{
    for (auto* button : { &option0_, &option1_, &option2_, &option3_ })
        addAndMakeVisible(*button);
}

void ActionsPanel::syncOptions()
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
            buttons[i]->onClick = nullptr;
        }
    }
}

void ActionsPanel::paint(juce::Graphics& g)
{
    syncOptions();
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(14), "ACTIONS");
    bounds.removeFromTop(6);
    Theme::drawBody(g, bounds.removeFromTop(18), "What do you want to change?");
}

void ActionsPanel::resized()
{
    syncOptions();
    auto bounds = getLocalBounds().reduced(16, 12);
    bounds.removeFromTop(48);
    const int gap = 8;
    const int h = juce::jmax(32, (bounds.getHeight() - gap * 3) / 4);
    ActionButton* buttons[] = { &option0_, &option1_, &option2_, &option3_ };
    for (auto* button : buttons)
    {
        if (!button->isVisible())
            continue;
        button->setBounds(bounds.removeFromTop(h));
        bounds.removeFromTop(gap);
    }
}

} // namespace sonora
