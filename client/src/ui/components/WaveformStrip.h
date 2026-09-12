#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class WaveformStrip : public juce::Component
{
public:
    explicit WaveformStrip(AppState& state);
    void paint(juce::Graphics& g) override;

private:
    AppState& state_;
};

} // namespace sonora
