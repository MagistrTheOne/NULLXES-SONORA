#include "ui/components/TopBar.h"

#include "ui/theme/Theme.h"

namespace sonora
{

TopBar::TopBar(AppState& state) : state_(state)
{
    bind(listen, WorkspaceTab::Listen);
    bind(improve, WorkspaceTab::Improve);
    bind(create, WorkspaceTab::Create);
    addAndMakeVisible(lab);
    lab.onClick = [this] {
        if (state_.labOpen())
            state_.closeLab();
        else
            state_.openLab();
    };
    addAndMakeVisible(soni);
    soni.onClick = [this] { state_.toggleSoni(); };
    addAndMakeVisible(advanced);
    advanced.onClick = [this] { state_.toggleUiMode(); };
}

void TopBar::bind(juce::TextButton& button, WorkspaceTab tab)
{
    addAndMakeVisible(button);
    button.onClick = [this, tab] { state_.setTab(tab); };
    style(button, state_.tab() == tab, true);
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
    style(listen, tab == WorkspaceTab::Listen, true);
    style(improve, tab == WorkspaceTab::Improve, true);
    style(create, tab == WorkspaceTab::Create, true);
    style(lab, state_.labOpen(), true);
    style(soni, state_.soniOpen(), true);
    advanced.setButtonText(state_.uiMode() == UiMode::Advanced ? "ADVANCED" : "SIMPLE");
    style(advanced, state_.uiMode() == UiMode::Advanced, true);

    auto bounds = getLocalBounds();
    auto brand = bounds.removeFromLeft(280);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawText("NULLXES SONORA V1.0.1", brand.removeFromTop(16), juce::Justification::centredLeft, true);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("SONORA", brand.removeFromTop(26), juce::Justification::centredLeft, true);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawText("ADAPTIVE SOUND INTELLIGENCE", brand, juce::Justification::centredLeft, true);

    auto hud = getLocalBounds().removeFromRight(320);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawText("FREE VST3", hud.removeFromTop(16), juce::Justification::centredRight, true);
    g.drawText("PREMIUM  SONI AI ASSISTANT", hud.removeFromTop(16), juce::Justification::centredRight, true);

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

void TopBar::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(290);
    auto right = bounds.removeFromRight(320);
    advanced.setBounds(right.removeFromBottom(22).removeFromRight(90));
    right.removeFromRight(8);
    soni.setBounds(right.removeFromBottom(22).removeFromRight(56));
    right.removeFromRight(8);
    lab.setBounds(right.removeFromBottom(22).removeFromRight(48));
    const int w = bounds.getWidth() / 3;
    listen.setBounds(bounds.removeFromLeft(w));
    improve.setBounds(bounds.removeFromLeft(w));
    create.setBounds(bounds);
}

} // namespace sonora
