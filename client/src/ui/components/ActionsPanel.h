#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class ActionsPanel : public juce::Component
{
public:
    explicit ActionsPanel(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void syncOptions();

    AppState& state_;
    ActionButton option0_;
    ActionButton option1_;
    ActionButton option2_;
    ActionButton option3_;
};

} // namespace sonora
