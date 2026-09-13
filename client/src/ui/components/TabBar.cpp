#include "ui/components/TabBar.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
void styleTab(juce::TextButton& button, bool active)
{
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::textColourOffId, active ? Theme::text() : Theme::muted());
    button.setColour(juce::TextButton::textColourOnId, Theme::text());
}
} // namespace

TabBar::TabBar(AppState& state) : state_(state)
{
    bind(listen, WorkspaceTab::Listen);
    bind(improve, WorkspaceTab::Improve);
    bind(create, WorkspaceTab::Create);
}

void TabBar::bind(juce::TextButton& button, WorkspaceTab tab)
{
    addAndMakeVisible(button);
    button.onClick = [this, tab] { state_.setTab(tab); };
    styleTab(button, state_.tab() == tab);
}

void TabBar::paint(juce::Graphics& g)
{
    const auto tab = state_.tab();
    styleTab(listen, tab == WorkspaceTab::Listen);
    styleTab(improve, tab == WorkspaceTab::Improve);
    styleTab(create, tab == WorkspaceTab::Create);

    juce::TextButton* active = &listen;
    if (tab == WorkspaceTab::Improve)
        active = &improve;
    else if (tab == WorkspaceTab::Create)
        active = &create;

    g.setColour(Theme::border());
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.setColour(Theme::accent());
    g.fillRect(active->getBounds().withHeight(1).withY(getHeight() - 1));
}

void TabBar::resized()
{
    auto row = getLocalBounds();
    const int w = row.getWidth() / 3;
    listen.setBounds(row.removeFromLeft(w));
    improve.setBounds(row.removeFromLeft(w));
    create.setBounds(row);
}

} // namespace sonora
