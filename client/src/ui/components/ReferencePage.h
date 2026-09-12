#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace sonora
{

class ReferencePage : public juce::Component
{
public:
    explicit ReferencePage(AppState& state);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void chooseReference();

    AppState& state_;
    ActionButton load_;
    std::unique_ptr<juce::FileChooser> chooser_;
};

} // namespace sonora
