#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace sonora
{
namespace
{
constexpr int kMaxCaptureSeconds = 180;
}

PluginProcessor::PluginProcessor()
    : juce::AudioProcessor(BusesProperties()
                               .withInput("Input", juce::AudioChannelSet::stereo(), true)
                               .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void PluginProcessor::prepareToPlay(double sampleRate, int)
{
    hostSr_ = sampleRate > 0.0 ? sampleRate : 44100.0;
    const int frames = juce::jmax(1, (int) std::llround(hostSr_ * (double) kMaxCaptureSeconds));
    capture_.setSize(2, frames, false, true, true);
    writeFrames_.store(0);
}

void PluginProcessor::releaseResources()
{
    listening_.store(false);
}

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

    if (!listening_.load(std::memory_order_acquire))
        return;

    const int destCh = juce::jmin(2, capture_.getNumChannels());
    const int srcCh = juce::jmax(1, juce::jmin(2, buffer.getNumChannels()));
    int write = writeFrames_.load(std::memory_order_relaxed);
    const int room = capture_.getNumSamples() - write;
    const int copy = juce::jmin(samples, room);
    if (copy <= 0)
    {
        listening_.store(false, std::memory_order_release);
        return;
    }

    for (int c = 0; c < destCh; ++c)
    {
        const float* src = buffer.getReadPointer(juce::jmin(c, srcCh - 1));
        capture_.copyFrom(c, write, src, copy);
    }
    writeFrames_.store(write + copy, std::memory_order_release);
}

void PluginProcessor::startListen()
{
    writeFrames_.store(0, std::memory_order_release);
    listening_.store(true, std::memory_order_release);
}

bool PluginProcessor::takeCapture(juce::AudioBuffer<float>& dest, double& sampleRate)
{
    listening_.store(false, std::memory_order_release);
    const int n = writeFrames_.exchange(0, std::memory_order_acq_rel);
    sampleRate = hostSr_;
    if (n < (int) std::llround(hostSr_ * 0.25))
        return false;
    dest.setSize(capture_.getNumChannels(), n, false, true, true);
    for (int c = 0; c < dest.getNumChannels(); ++c)
        dest.copyFrom(c, 0, capture_, c, 0, n);
    return true;
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
