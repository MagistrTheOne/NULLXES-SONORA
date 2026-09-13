#pragma once

#include "state/AppState.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include <atomic>
#include <mutex>

namespace sonora
{

class PluginProcessor;

class EngineHost : private juce::Timer
{
public:
    static EngineHost& get();

    AppState& session() { return session_; }

    void installHooks();
    void prepare(double sampleRate);
    void setEar(PluginProcessor* who);
    void process(PluginProcessor* who, juce::AudioBuffer<float>& buffer, juce::AudioPlayHead* playHead);
    bool isListening(const PluginProcessor* who) const;
    void startListen(PluginProcessor* who);
    void finishListen();

private:
    EngineHost();
    void timerCallback() override;
    void writeRing(const juce::AudioBuffer<float>& buffer);
    void snapshotLast(juce::AudioBuffer<float>& dest, int frames) const;
    void pullLiveMeters();
    void kickAnalysis();

    AppState session_;
    juce::AudioBuffer<float> ring_;
    std::atomic<int> writePos_ { 0 };
    std::atomic<int> filled_ { 0 };
    std::atomic<PluginProcessor*> ear_ { nullptr };
    std::atomic<bool> hostPlaying_ { false };
    std::atomic<int> hostBpmX100_ { 0 };
    std::atomic<int> hostPpqX100_ { 0 };
    std::atomic<int> hostSecX100_ { 0 };
    std::atomic<double> hostSr_ { 44100.0 };
    double allocatedSr_ = 0.0;
    int ringFrames_ = 0;
    bool hooks_ = false;
    bool timerOn_ = false;
    int tick_ = 0;
    int lastSpokenAnalysis_ = 0;
};

} // namespace sonora
