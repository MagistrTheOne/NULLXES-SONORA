#pragma once

#include "models/AudioAnalysis.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class SpectrumView : public juce::Component
{
public:
    SpectrumView();

    void setBands(const models::FrequencyDistribution& bands);
    void clear();

    void paint(juce::Graphics& g) override;

private:
    models::FrequencyDistribution bands_;
    bool hasData_ = false;
};

} // namespace sonora
