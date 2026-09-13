#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class WaveformStrip : public juce::Component,
                      private juce::Timer
{
public:
    explicit WaveformStrip(AppState& state);
    ~WaveformStrip() override;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    juce::Rectangle<int> playBounds() const;
    juce::Rectangle<int> waveBounds() const;
    void seekFrom(const juce::MouseEvent& event);

    AppState& state_;
    float phase_ = 0.0f;
};

} // namespace sonora
