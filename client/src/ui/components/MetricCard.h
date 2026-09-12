#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class MetricCard : public juce::Component
{
public:
    MetricCard();

    void setLabel(const juce::String& label);
    void setValue(const juce::String& value);
    void setHint(const juce::String& hint);
    void setEmpty(bool empty);

    void paint(juce::Graphics& g) override;

private:
    juce::String label_;
    juce::String value_ { "---" };
    juce::String hint_;
    bool empty_ = true;
};

} // namespace sonora
