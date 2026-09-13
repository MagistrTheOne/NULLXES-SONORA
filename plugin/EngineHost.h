#pragma once

#include "state/AppState.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <atomic>
#include <mutex>

namespace sonora
{

class PluginProcessor;

class EngineHost
{
public:
    static EngineHost& get();

    AppState& session() { return session_; }

    void installHooks();
    void prepare(double sampleRate);
    void startListen(PluginProcessor* who);
    void process(PluginProcessor* who, juce::AudioBuffer<float>& buffer);
    bool isListening(const PluginProcessor* who) const;
    void finishListen();

private:
    EngineHost() = default;

    AppState session_;
    juce::AudioBuffer<float> capture_;
    std::mutex prepareLock_;
    std::atomic<PluginProcessor*> listener_ { nullptr };
    std::atomic<bool> listening_ { false };
    std::atomic<int> writeFrames_ { 0 };
    std::atomic<double> hostSr_ { 44100.0 };
    double allocatedSr_ = 0.0;
    bool hooks_ = false;
};

} // namespace sonora
