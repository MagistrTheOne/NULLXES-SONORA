#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class Dashboard : public juce::Component
{
public:
    explicit Dashboard(AppState& state);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    AppState& state_;
};

} // namespace sonora
