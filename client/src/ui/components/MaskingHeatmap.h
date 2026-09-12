#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class MaskingHeatmap : public juce::Component
{
public:
    explicit MaskingHeatmap(AppState& state);
    void paint(juce::Graphics& g) override;

private:
    AppState& state_;
};

} // namespace sonora
