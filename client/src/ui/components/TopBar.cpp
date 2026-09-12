#include "ui/components/TopBar.h"

#include "ui/theme/Theme.h"

namespace sonora
{

TopBar::TopBar(AppState& state) : state_(state)
{
    bind(track, WorkspaceTab::Track, true);
    bind(mix, WorkspaceTab::Mix, true);
    bind(arrangement, WorkspaceTab::Arrangement, true);
    bind(create, WorkspaceTab::Create, true);
    bind(reference, WorkspaceTab::Reference, false);
    addAndMakeVisible(advanced);
    advanced.onClick = [this] { state_.toggleUiMode(); };
}

void TopBar::bind(juce::TextButton& button, WorkspaceTab tab, bool enabled)
{
    addAndMakeVisible(button);
    if (enabled)
        button.onClick = [this, tab] { state_.setTab(tab); };
    style(button, state_.tab() == tab, enabled);
}

void TopBar::style(juce::TextButton& button, bool active, bool enabled)
{
    button.setEnabled(enabled);
    button.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    button.setColour(juce::TextButton::textColourOffId, !enabled ? Theme::muted().withAlpha(0.35f)
                                                                : (active ? Theme::text() : Theme::muted()));
    button.setColour(juce::TextButton::textColourOnId, Theme::text());
}

void TopBar::paint(juce::Graphics& g)
{
    const auto tab = state_.tab();
    style(track, tab == WorkspaceTab::Track, true);
    style(mix, tab == WorkspaceTab::Mix, true);
    style(arrangement, tab == WorkspaceTab::Arrangement, true);
    style(create, tab == WorkspaceTab::Create, true);
    style(reference, false, false);
    advanced.setButtonText(state_.uiMode() == UiMode::Advanced ? "ADVANCED" : "SIMPLE");
    style(advanced, state_.uiMode() == UiMode::Advanced, true);

    auto bounds = getLocalBounds();
    auto brand = bounds.removeFromLeft(220);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawText("NULLXES", brand.removeFromTop(16), juce::Justification::centredLeft, true);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("SONORA", brand.removeFromTop(26), juce::Justification::centredLeft, true);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawText("ADAPTIVE SOUND INTELLIGENCE", brand, juce::Justification::centredLeft, true);

    auto hud = getLocalBounds().removeFromRight(280);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawText("SESSION 001", hud.removeFromTop(16), juce::Justification::centredRight, true);
    g.drawText("LOCAL MODE", hud.removeFromTop(16), juce::Justification::centredRight, true);

    juce::TextButton* active = &track;
    switch (tab)
    {
        case WorkspaceTab::Mix: active = &mix; break;
        case WorkspaceTab::Arrangement: active = &arrangement; break;
        case WorkspaceTab::Create: active = &create; break;
        default: break;
    }
    g.setColour(Theme::border());
    g.fillRect(0, getHeight() - 1, getWidth(), 1);
    g.setColour(Theme::accent());
    g.fillRect(active->getBounds().withHeight(1).withY(getHeight() - 1));
}

void TopBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(240);
    auto right = bounds.removeFromRight(280);
    advanced.setBounds(right.removeFromBottom(22).removeFromRight(90));
    const int w = bounds.getWidth() / 5;
    track.setBounds(bounds.removeFromLeft(w));
    mix.setBounds(bounds.removeFromLeft(w));
    arrangement.setBounds(bounds.removeFromLeft(w));
    create.setBounds(bounds.removeFromLeft(w));
    reference.setBounds(bounds);
}

} // namespace sonora
