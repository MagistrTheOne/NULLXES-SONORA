#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class LabOverlay : public juce::Component
{
public:
    explicit LabOverlay(AppState& state);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    void syncOptions();
    juce::Rectangle<int> cardBounds() const;

    AppState& state_;
    ActionButton option0_;
    ActionButton option1_;
    ActionButton option2_;
    ActionButton option3_;
};

} // namespace sonora
