#pragma once

#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class AssistantPanel : public juce::Component
{
public:
    AssistantPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::TextEditor input_;
    ActionButton chords_;
    ActionButton bassline_;
    ActionButton pad_;
    ActionButton arrangement_;
};

} // namespace sonora
