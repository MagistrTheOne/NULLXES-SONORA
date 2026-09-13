#include "EngineHost.h"
#include "PluginProcessor.h"

#include <cmath>

namespace sonora
{
namespace
{
constexpr int kRingSeconds = 24;
constexpr int kAnalyzeSeconds = 16;
constexpr int kAnalyzeEveryTicks = 15; // 15 * 200ms = 3s
}

EngineHost& EngineHost::get()
{
    static EngineHost host;
    return host;
}

EngineHost::EngineHost()
{
    session_.setDawHost(true);
}

void EngineHost::installHooks()
{
    if (hooks_)
        return;
    hooks_ = true;
    session_.setDawHost(true);
    session_.setCaptureHooks(
        [this] { startListen(static_cast<PluginProcessor*>(session_.captureToken())); },
        [this] { finishListen(); });
    if (!timerOn_)
    {
        timerOn_ = true;
        startTimer(200);
    }
}

void EngineHost::prepare(double sampleRate)
{
    const double sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    hostSr_.store(sr, std::memory_order_release);
    const int frames = juce::jmax(1, (int) std::llround(sr * (double) kRingSeconds));
    if (std::abs(sr - allocatedSr_) < 0.5 && ring_.getNumSamples() == frames)
        return;
    allocatedSr_ = sr;
    ringFrames_ = frames;
    ring_.setSize(2, frames, false, true, true);
    writePos_.store(0, std::memory_order_release);
    filled_.store(0, std::memory_order_release);
}

void EngineHost::setEar(PluginProcessor* who)
{
    if (who != nullptr)
        ear_.store(who, std::memory_order_release);
}

void EngineHost::writeRing(const juce::AudioBuffer<float>& buffer)
{
    if (ringFrames_ <= 0)
        return;
    const int n = buffer.getNumSamples();
    const int srcCh = juce::jmax(1, juce::jmin(2, buffer.getNumChannels()));
    int w = writePos_.load(std::memory_order_relaxed);
    for (int i = 0; i < n; ++i)
    {
        ring_.setSample(0, w, buffer.getSample(0, i));
        ring_.setSample(1, w, buffer.getSample(juce::jmin(1, srcCh - 1), i));
        w += 1;
        if (w >= ringFrames_)
            w = 0;
    }
    writePos_.store(w, std::memory_order_release);
    const int have = juce::jmin(ringFrames_, filled_.load(std::memory_order_relaxed) + n);
    filled_.store(have, std::memory_order_release);
}

void EngineHost::process(PluginProcessor* who, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead* playHead)
{
    bool playing = false;
    bool haveHead = false;
    double bpm = 0.0, ppq = 0.0, sec = 0.0;
    if (playHead != nullptr)
    {
        if (auto pos = playHead->getPosition())
        {
            haveHead = true;
            playing = pos->getIsPlaying();
            bpm = pos->getBpm().orFallback(0.0);
            ppq = pos->getPpqPosition().orFallback(0.0);
            sec = pos->getTimeInSeconds().orFallback(0.0);
        }
    }
    hostPlaying_.store(playing, std::memory_order_release);
    hostBpmX100_.store((int) std::lround(bpm * 100.0), std::memory_order_relaxed);
    hostPpqX100_.store((int) std::lround(ppq * 100.0), std::memory_order_relaxed);
    hostSecX100_.store((int) std::lround(sec * 100.0), std::memory_order_relaxed);

    auto* ear = ear_.load(std::memory_order_acquire);
    if (ear == nullptr)
        ear_.compare_exchange_strong(ear, who);
    if (ear_.load(std::memory_order_acquire) != who)
        return;

    bool hear = playing;
    if (!haveHead)
    {
        float peak = 0.0f;
        for (int c = 0; c < buffer.getNumChannels(); ++c)
            peak = juce::jmax(peak, buffer.getMagnitude(c, 0, buffer.getNumSamples()));
        hear = peak > 1.0e-4f;
    }
    if (hear)
        writeRing(buffer);
}

void EngineHost::snapshotLast(juce::AudioBuffer<float>& dest, int frames) const
{
    const int have = filled_.load(std::memory_order_acquire);
    const int cap = ring_.getNumSamples();
    if (have <= 0 || cap <= 0)
    {
        dest.setSize(2, 0);
        return;
    }
    const int n = juce::jmin(frames, have, cap);
    dest.setSize(2, n, false, true, true);
    int w = writePos_.load(std::memory_order_acquire);
    int start = w - n;
    if (start < 0)
        start += cap;
    for (int i = 0; i < n; ++i)
    {
        const int idx = (start + i) % cap;
        dest.setSample(0, i, ring_.getSample(0, idx));
        dest.setSample(1, i, ring_.getSample(1, idx));
    }
}

void EngineHost::pullLiveMeters()
{
    const double sr = hostSr_.load(std::memory_order_acquire);
    const int window = juce::jmax(256, (int) std::lround(sr * 0.25));
    juce::AudioBuffer<float> snap;
    snapshotLast(snap, window);
    float energy = 0.0f, peak = 0.0f, stereo = 0.0f;
    if (snap.getNumSamples() > 0)
    {
        double acc = 0.0;
        for (int i = 0; i < snap.getNumSamples(); ++i)
        {
            const float l = snap.getSample(0, i);
            const float r = snap.getSample(1, i);
            acc += (double) l * l + (double) r * r;
            peak = juce::jmax(peak, std::abs(l), std::abs(r));
            stereo += std::abs(l - r);
        }
        energy = (float) std::sqrt(acc / (double) (snap.getNumSamples() * 2));
        stereo /= (float) snap.getNumSamples();
    }
    session_.updateTransport(
        hostPlaying_.load(std::memory_order_acquire),
        (double) hostBpmX100_.load(std::memory_order_relaxed) / 100.0,
        (double) hostPpqX100_.load(std::memory_order_relaxed) / 100.0,
        (double) hostSecX100_.load(std::memory_order_relaxed) / 100.0);
    session_.updateLiveMeters(energy, peak, stereo);
}

void EngineHost::kickAnalysis()
{
    if (session_.busy())
        return;
    const double sr = hostSr_.load(std::memory_order_acquire);
    const int need = juce::jmax(1, (int) std::lround(sr * 2.0));
    if (filled_.load(std::memory_order_acquire) < need)
        return;
    juce::AudioBuffer<float> snap;
    snapshotLast(snap, (int) std::lround(sr * (double) kAnalyzeSeconds));
    if (snap.getNumSamples() < need)
        return;
    session_.analyzeLive(std::move(snap), sr, "FL session");
}

void EngineHost::timerCallback()
{
    pullLiveMeters();
    ++tick_;
    const bool playing = hostPlaying_.load(std::memory_order_acquire);
    if (playing && tick_ % kAnalyzeEveryTicks == 0)
        kickAnalysis();
}

void EngineHost::startListen(PluginProcessor* who)
{
    setEar(who);
}

void EngineHost::finishListen() {}

bool EngineHost::isListening(const PluginProcessor* who) const
{
    return hostPlaying_.load(std::memory_order_acquire) && ear_.load(std::memory_order_acquire) == who;
}

} // namespace sonora
