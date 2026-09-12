#include "ui/components/TabBar.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
void styleTab(juce::TextButton& button, bool active, bool enabled)
{
    button.setEnabled(enabled);
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::textColourOffId, !enabled ? Theme::muted().withAlpha(0.45f)
                                                                : (active ? Theme::text() : Theme::muted()));
    button.setColour(juce::TextButton::textColourOnId, Theme::text());
}
} // namespace

TabBar::TabBar(AppState& state) : state_(state)
{
    generate.setButtonText("CREATE");
    bind(overview, WorkspaceTab::Overview, true);
    bind(spectrum, WorkspaceTab::Spectrum, true);
    bind(harmony, WorkspaceTab::Harmony, true);
    bind(generate, WorkspaceTab::Generate, true);
    bind(mix, WorkspaceTab::Mix, false);
    bind(master, WorkspaceTab::Master, false);
}

void TabBar::bind(juce::TextButton& button, WorkspaceTab tab, bool enabled)
{
    addAndMakeVisible(button);
    if (enabled)
        button.onClick = [this, tab] { state_.setTab(tab); };
    styleTab(button, state_.tab() == tab, enabled);
}

void TabBar::paint(juce::Graphics& g)
{
    styleTab(overview, state_.tab() == WorkspaceTab::Overview, true);
    styleTab(spectrum, state_.tab() == WorkspaceTab::Spectrum, true);
    styleTab(harmony, state_.tab() == WorkspaceTab::Harmony, true);
    styleTab(generate, state_.tab() == WorkspaceTab::Generate, true);
    styleTab(mix, false, false);
    styleTab(master, false, false);

    juce::TextButton* active = &overview;
    switch (state_.tab())
    {
        case WorkspaceTab::Spectrum: active = &spectrum; break;
        case WorkspaceTab::Harmony: active = &harmony; break;
        case WorkspaceTab::Generate: active = &generate; break;
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
    const int live = (row.getWidth() * 4) / 6;
    const int dead = row.getWidth() - live;
    auto active = row.removeFromLeft(live);
    const int w = active.getWidth() / 4;
    overview.setBounds(active.removeFromLeft(w));
    spectrum.setBounds(active.removeFromLeft(w));
    harmony.setBounds(active.removeFromLeft(w));
    generate.setBounds(active);
    mix.setBounds(row.removeFromLeft(dead / 2));
    master.setBounds(row);
}

} // namespace sonora
