#pragma once

#include "state/AppState.h"

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
    juce::TextButton chords { "CHORDS" };
    juce::TextButton bassline { "BASSLINE" };
    juce::TextButton pad { "PAD" };
    juce::TextButton arrangement { "ARRANGEMENT" };
};

} // namespace sonora
