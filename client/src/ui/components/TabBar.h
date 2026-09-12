#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class TabBar : public juce::Component
{
public:
    explicit TabBar(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
    juce::TextButton overview { "OVERVIEW" };
    juce::TextButton spectrum { "SPECTRUM" };
    juce::TextButton harmony { "HARMONY" };
    juce::TextButton generate { "GENERATE" };
    juce::TextButton mix { "MIX" };
    juce::TextButton master { "MASTER" };

    void bind(juce::TextButton& button, WorkspaceTab tab, bool enabled);
};

} // namespace sonora
