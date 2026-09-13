#include "PluginProcessor.h"
#include "EngineHost.h"
#include "PluginEditor.h"

namespace sonora
{

PluginProcessor::PluginProcessor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void PluginProcessor::prepareToPlay(double sampleRate, int)
{
    EngineHost::get().prepare(sampleRate);
}

void PluginProcessor::releaseResources() {}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    const auto in = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return in == out || in == juce::AudioChannelSet::disabled();
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;

    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < channels; ++ch)
        buffer.clear(ch, 0, samples);

    EngineHost::get().process(this, buffer, getPlayHead());
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto json = EngineHost::get().session().toSessionJson();
    destData.setSize(0);
    destData.append(json.toRawUTF8(), json.getNumBytesAsUTF8());
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;
    EngineHost::get().session().applySessionJson(
        juce::String::fromUTF8(static_cast<const char*>(data), sizeInBytes));
}

bool PluginProcessor::isListening() const
{
    return EngineHost::get().isListening(this);
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

} // namespace sonora

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new sonora::PluginProcessor();
}
