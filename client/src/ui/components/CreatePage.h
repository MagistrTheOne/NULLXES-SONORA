#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class CreatePage : public juce::Component
{
public:
    explicit CreatePage(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
    ActionButton chords_;
    ActionButton bass_;
    ActionButton pad_;
    ActionButton arrangement_;
    ActionButton vocal_;
};

} // namespace sonora
