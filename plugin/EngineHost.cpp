#include "EngineHost.h"
#include "PluginProcessor.h"

#include <cmath>

namespace sonora
{
namespace
{
constexpr int kMaxCaptureSeconds = 180;
}

EngineHost& EngineHost::get()
{
    static EngineHost host;
    return host;
}

void EngineHost::installHooks()
{
    if (hooks_)
        return;
    hooks_ = true;
    session_.setCaptureHooks(
        [this] {
            startListen(static_cast<PluginProcessor*>(session_.captureToken()));
        },
        [this] { finishListen(); });
}

void EngineHost::prepare(double sampleRate)
{
    const double sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    std::lock_guard<std::mutex> lock(prepareLock_);
    if (std::abs(sr - allocatedSr_) < 0.5 && capture_.getNumSamples() > 0)
    {
        hostSr_.store(sr, std::memory_order_release);
        return;
    }
    allocatedSr_ = sr;
    hostSr_.store(sr, std::memory_order_release);
    const int frames = juce::jmax(1, (int) std::llround(sr * (double) kMaxCaptureSeconds));
    capture_.setSize(2, frames, false, true, true);
    writeFrames_.store(0, std::memory_order_release);
}

void EngineHost::startListen(PluginProcessor* who)
{
    if (who == nullptr)
        return;
    writeFrames_.store(0, std::memory_order_release);
    listener_.store(who, std::memory_order_release);
    listening_.store(true, std::memory_order_release);
}

void EngineHost::process(PluginProcessor* who, juce::AudioBuffer<float>& buffer)
{
    if (!listening_.load(std::memory_order_acquire) || listener_.load(std::memory_order_acquire) != who)
        return;

    const int destCh = juce::jmin(2, capture_.getNumChannels());
    const int srcCh = juce::jmax(1, juce::jmin(2, buffer.getNumChannels()));
    int write = writeFrames_.load(std::memory_order_relaxed);
    const int room = capture_.getNumSamples() - write;
    const int copy = juce::jmin(buffer.getNumSamples(), room);
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

bool EngineHost::isListening(const PluginProcessor* who) const
{
    return listening_.load(std::memory_order_acquire) && listener_.load(std::memory_order_acquire) == who;
}

void EngineHost::finishListen()
{
    listening_.store(false, std::memory_order_release);
    listener_.store(nullptr, std::memory_order_release);
    const int n = writeFrames_.exchange(0, std::memory_order_acq_rel);
    const double sr = hostSr_.load(std::memory_order_acquire);
    if (n < (int) std::llround(sr * 0.25) || capture_.getNumSamples() <= 0)
    {
        session_.failListen("Play the track in the DAW, then press STOP.");
        return;
    }
    juce::AudioBuffer<float> dest(capture_.getNumChannels(), n);
    for (int c = 0; c < dest.getNumChannels(); ++c)
        dest.copyFrom(c, 0, capture_, c, 0, n);
    session_.analyzeBuffer(std::move(dest), sr, "DAW capture");
}

} // namespace sonora
