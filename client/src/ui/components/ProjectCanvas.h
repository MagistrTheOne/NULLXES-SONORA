#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class ProjectCanvas : public juce::Component
{
public:
    explicit ProjectCanvas(AppState& state);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    AppState& state_;
    juce::Rectangle<int> nodeBounds(int index) const;
};

} // namespace sonora
