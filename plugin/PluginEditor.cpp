#include "PluginEditor.h"
#include "EngineHost.h"

namespace sonora
{

PluginEditor::PluginEditor(PluginProcessor& processor)
    : juce::AudioProcessorEditor(processor)
    , processor_(processor)
    , state_(EngineHost::get().session())
    , dashboard_(state_)
{
    EngineHost::get().installHooks();
    EngineHost::get().setEar(&processor_);
    dashboard_.setCaptureSite([this] { return (void*) &processor_; });

    setOpaque(true);
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
