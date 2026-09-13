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
    juce::TextButton listen { "LISTEN" };
    juce::TextButton improve { "IMPROVE" };
    juce::TextButton create { "CREATE" };

    void bind(juce::TextButton& button, WorkspaceTab tab);
};

} // namespace sonora
