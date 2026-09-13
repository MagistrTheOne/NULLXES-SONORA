#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"
#include "ui/components/SoniFace.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class SoniMeet : public juce::Component,
                 private juce::ChangeListener
{
public:
    explicit SoniMeet(AppState& state);
    ~SoniMeet() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;

    AppState& state_;
    SoniFace face_;
    ActionButton enter_;
};

} // namespace sonora
