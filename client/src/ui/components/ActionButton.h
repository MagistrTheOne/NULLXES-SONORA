#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class ActionButton : public juce::Component
{
public:
    enum class Variant
    {
        Outline,
        Ghost
    };

    ActionButton();

    void setLabel(const juce::String& label);
    void setVariant(Variant variant);
    std::function<void()> onClick;

    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    juce::String label_;
    Variant variant_ { Variant::Outline };
    bool hover_ = false;
};

} // namespace sonora
