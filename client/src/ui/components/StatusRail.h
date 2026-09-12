#pragma once

#include "state/AppState.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class StatusRail : public juce::Component
{
public:
    StatusRail();

    void setState(ComputeState state);
    void setUsage(const juce::String& usage);
    void setResult(const juce::String& result);

    void paint(juce::Graphics& g) override;

private:
    ComputeState state_ { ComputeState::Offline };
    juce::String usage_ { "---" };
    juce::String result_ { "---" };
};

} // namespace sonora
