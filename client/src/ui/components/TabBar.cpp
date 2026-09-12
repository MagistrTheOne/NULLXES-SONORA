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
    generate.setButtonText("CREATE");
    bind(overview, WorkspaceTab::Overview);
    bind(dna, WorkspaceTab::Dna);
    bind(structure, WorkspaceTab::Structure);
    bind(spectrum, WorkspaceTab::Spectrum);
    bind(generate, WorkspaceTab::Generate);
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
    styleTab(overview, tab == WorkspaceTab::Overview);
    styleTab(dna, tab == WorkspaceTab::Dna);
    styleTab(structure, tab == WorkspaceTab::Structure);
    styleTab(spectrum, tab == WorkspaceTab::Spectrum);
    styleTab(generate, tab == WorkspaceTab::Generate || tab == WorkspaceTab::Harmony);

    juce::TextButton* active = &overview;
    switch (tab)
    {
        case WorkspaceTab::Dna: active = &dna; break;
        case WorkspaceTab::Structure: active = &structure; break;
        case WorkspaceTab::Spectrum: active = &spectrum; break;
        case WorkspaceTab::Generate:
        case WorkspaceTab::Harmony: active = &generate; break;
        default: break;
    }

    g.setColour(Theme::border());
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.setColour(Theme::accent());
    g.fillRect(active->getBounds().withHeight(1).withY(getHeight() - 1));
}

void TabBar::resized()
{
    auto row = getLocalBounds();
    const int w = row.getWidth() / 5;
    overview.setBounds(row.removeFromLeft(w));
    dna.setBounds(row.removeFromLeft(w));
    structure.setBounds(row.removeFromLeft(w));
    spectrum.setBounds(row.removeFromLeft(w));
    generate.setBounds(row);
}

} // namespace sonora
