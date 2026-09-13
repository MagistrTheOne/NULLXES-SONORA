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
    bind(understand, WorkspaceTab::Understand);
    bind(create, WorkspaceTab::Create);
    bind(soni, WorkspaceTab::Soni);
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
    styleTab(understand, tab == WorkspaceTab::Understand);
    styleTab(create, tab == WorkspaceTab::Create);
    styleTab(soni, tab == WorkspaceTab::Soni);

    juce::TextButton* active = &listen;
    if (tab == WorkspaceTab::Understand)
        active = &understand;
    else if (tab == WorkspaceTab::Create)
        active = &create;
    else if (tab == WorkspaceTab::Soni)
        active = &soni;

    g.setColour(Theme::border());
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.setColour(Theme::accent());
    g.fillRect(active->getBounds().withHeight(1).withY(getHeight() - 1));
}

void TabBar::resized()
{
    auto row = getLocalBounds();
    const int w = row.getWidth() / 4;
    listen.setBounds(row.removeFromLeft(w));
    understand.setBounds(row.removeFromLeft(w));
    create.setBounds(row.removeFromLeft(w));
    soni.setBounds(row);
}

} // namespace sonora
