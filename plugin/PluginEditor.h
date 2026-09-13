#pragma once

#include "PluginProcessor.h"
#include "state/AppState.h"
#include "ui/screens/Dashboard.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace sonora
{

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor(PluginProcessor& processor);
    ~PluginEditor() override = default;

    void resized() override;

private:
    PluginProcessor& processor_;
    AppState& state_;
    Dashboard dashboard_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace sonora
