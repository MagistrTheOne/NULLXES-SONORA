#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class CreatePanel : public juce::Component
{
public:
    explicit CreatePanel(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
    ActionButton chords_;
    ActionButton bassline_;
    ActionButton arrangement_;
    ActionButton midi_;
};

} // namespace sonora
