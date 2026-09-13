#pragma once

#include "state/AppState.h"
#include "ui/components/SoniFace.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class LiveListenView : public juce::Component,
                       private juce::Timer
{
public:
    explicit LiveListenView(AppState& state);
    ~LiveListenView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    AppState& state_;
    SoniFace face_;
    float pulse_ = 0.0f;
};

} // namespace sonora
