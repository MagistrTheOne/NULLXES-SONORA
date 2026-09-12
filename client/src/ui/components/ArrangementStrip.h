#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class ArrangementStrip : public juce::Component
{
public:
    explicit ArrangementStrip(AppState& state);
    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    AppState& state_;
};

} // namespace sonora
