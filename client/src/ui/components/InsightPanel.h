#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class InsightPanel : public juce::Component
{
public:
    explicit InsightPanel(AppState& state);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
    ActionButton generate_;
    ActionButton createEq_;
};

} // namespace sonora
