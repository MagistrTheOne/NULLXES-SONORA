#include "PluginEditor.h"

namespace sonora
{

PluginEditor::PluginEditor(PluginProcessor& processor)
    : juce::AudioProcessorEditor(processor)
    , processor_(processor)
{
    setOpaque(true);
    state_.setCaptureHooks(
        [this] { processor_.startListen(); },
        [this] {
            juce::AudioBuffer<float> buffer;
            double sampleRate = 44100.0;
            if (processor_.takeCapture(buffer, sampleRate))
                state_.analyzeBuffer(std::move(buffer), sampleRate, "DAW capture");
            else
                state_.failListen("Play the track in the DAW, then press STOP.");
        });

    addAndMakeVisible(dashboard_);
    setResizeLimits(1280, 800, 4096, 2160);
    setResizable(true, false);
    setSize(1600, 960);
}

void PluginEditor::resized()
{
    dashboard_.setBounds(getLocalBounds());
}

} // namespace sonora
