#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace sonora
{

class CreatePage : public juce::Component
{
public:
    explicit CreatePage(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void exportMidi();

    AppState& state_;
    ActionButton chords_;
    ActionButton bass_;
    ActionButton drop_;
    ActionButton arrangement_;
    ActionButton midi_;
    std::unique_ptr<juce::FileChooser> chooser_;
};

} // namespace sonora
