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
    bind(overview, WorkspaceTab::Overview);
    bind(spectrum, WorkspaceTab::Spectrum);
    bind(harmony, WorkspaceTab::Harmony);
    bind(generate, WorkspaceTab::Generate);
    bind(mix, WorkspaceTab::Mix);
    bind(master, WorkspaceTab::Master);
}

void TabBar::bind(juce::TextButton& button, WorkspaceTab tab)
{
    addAndMakeVisible(button);
    button.onClick = [this, tab] { state_.setTab(tab); };
    styleTab(button, state_.tab() == tab);
}

void TabBar::paint(juce::Graphics& g)
{
    styleTab(overview, state_.tab() == WorkspaceTab::Overview);
    styleTab(spectrum, state_.tab() == WorkspaceTab::Spectrum);
    styleTab(harmony, state_.tab() == WorkspaceTab::Harmony);
    styleTab(generate, state_.tab() == WorkspaceTab::Generate);
    styleTab(mix, state_.tab() == WorkspaceTab::Mix);
    styleTab(master, state_.tab() == WorkspaceTab::Master);

    const auto active = [this] {
        switch (state_.tab())
        {
            case WorkspaceTab::Overview: return &overview;
            case WorkspaceTab::Spectrum: return &spectrum;
            case WorkspaceTab::Harmony: return &harmony;
            case WorkspaceTab::Generate: return &generate;
            case WorkspaceTab::Mix: return &mix;
            case WorkspaceTab::Master: return &master;
        }
        return &overview;
    }();

    g.setColour(Theme::accent());
    g.fillRect(active->getBounds().withHeight(1).withY(getHeight() - 1));
}

void TabBar::resized()
{
    auto row = getLocalBounds();
    const int w = row.getWidth() / 6;
    overview.setBounds(row.removeFromLeft(w));
    spectrum.setBounds(row.removeFromLeft(w));
    harmony.setBounds(row.removeFromLeft(w));
    generate.setBounds(row.removeFromLeft(w));
    mix.setBounds(row.removeFromLeft(w));
    master.setBounds(row);
}

} // namespace sonora
