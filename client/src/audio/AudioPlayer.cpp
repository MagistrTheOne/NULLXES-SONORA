#include "audio/AudioPlayer.h"

#include "backend/ClientLog.h"

namespace sonora
{

AudioPlayer::AudioPlayer()
{
    formats_.registerBasicFormats();
    const auto error = devices_.initialiseWithDefaultDevices(0, 2);
    deviceOk_ = error.isEmpty();
    if (!deviceOk_)
        clientLog("AudioPlayer device init failed: " + error);
    else
        attach();
}

AudioPlayer::~AudioPlayer()
{
    transport_.stop();
    transport_.setSource(nullptr);
    source_.reset();
    player_.setSource(nullptr);
    if (attached_)
        devices_.removeAudioCallback(&player_);
    devices_.closeAudioDevice();
}

void AudioPlayer::attach()
{
    if (attached_ || !deviceOk_)
        return;
    devices_.addAudioCallback(&player_);
    player_.setSource(&transport_);
    attached_ = true;
}

bool AudioPlayer::load(const juce::File& file)
{
    transport_.stop();
    transport_.setSource(nullptr);
    source_.reset();
    if (!file.existsAsFile())
        return false;

    auto* reader = formats_.createReaderFor(file);
    if (reader == nullptr)
    {
        clientLog("AudioPlayer cannot decode " + file.getFileName());
        return false;
    }

    source_ = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transport_.setSource(source_.get(), 0, nullptr, reader->sampleRate);
    transport_.setPosition(0.0);
    return true;
}

void AudioPlayer::unload()
{
    transport_.stop();
    transport_.setSource(nullptr);
    source_.reset();
}

void AudioPlayer::play()
{
    if (!deviceOk_ || source_ == nullptr)
        return;
    if (transport_.getCurrentPosition() >= transport_.getLengthInSeconds() - 0.05)
        transport_.setPosition(0.0);
    transport_.start();
}

void AudioPlayer::pause()
{
    transport_.stop();
}

void AudioPlayer::toggle()
{
    if (isPlaying())
        pause();
    else
        play();
}

void AudioPlayer::seekNormalized(float amount)
{
    if (source_ == nullptr)
        return;
    const auto length = transport_.getLengthInSeconds();
    if (length <= 0.0)
        return;
    transport_.setPosition(juce::jlimit(0.0, length, (double) amount * length));
}

bool AudioPlayer::isPlaying() const
{
    return transport_.isPlaying();
}

bool AudioPlayer::isReady() const
{
    return source_ != nullptr && transport_.getLengthInSeconds() > 0.0;
}

double AudioPlayer::positionSeconds() const
{
    return source_ == nullptr ? 0.0 : transport_.getCurrentPosition();
}

double AudioPlayer::lengthSeconds() const
{
    return source_ == nullptr ? 0.0 : transport_.getLengthInSeconds();
}

float AudioPlayer::positionNormalized() const
{
    const auto length = lengthSeconds();
    if (length <= 0.0)
        return 0.0f;
    return (float) juce::jlimit(0.0, 1.0, positionSeconds() / length);
}

} // namespace sonora
