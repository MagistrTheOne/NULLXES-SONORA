#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <memory>

namespace sonora
{

class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    bool load(const juce::File& file);
    void unload();
    void play();
    void pause();
    void toggle();
    void seekNormalized(float amount);
    bool isPlaying() const;
    bool isReady() const;
    bool deviceOk() const { return deviceOk_; }
    double positionSeconds() const;
    double lengthSeconds() const;
    float positionNormalized() const;

private:
    void attach();

    juce::AudioDeviceManager devices_;
    juce::AudioFormatManager formats_;
    juce::AudioSourcePlayer player_;
    juce::AudioTransportSource transport_;
    std::unique_ptr<juce::AudioFormatReaderSource> source_;
    bool deviceOk_ = false;
    bool attached_ = false;
};

} // namespace sonora
