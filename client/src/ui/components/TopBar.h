#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class TopBar : public juce::Component
{
public:
    explicit TopBar(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
    juce::TextButton track { "TRACK" };
    juce::TextButton mix { "MIX" };
    juce::TextButton arrangement { "ARRANGEMENT" };
    juce::TextButton create { "CREATE" };
    juce::TextButton reference { "REFERENCE" };
    juce::TextButton advanced { "SIMPLE" };

    void bind(juce::TextButton& button, WorkspaceTab tab, bool enabled);
    void style(juce::TextButton& button, bool active, bool enabled);
};

} // namespace sonora
