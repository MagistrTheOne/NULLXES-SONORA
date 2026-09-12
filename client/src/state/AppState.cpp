#include "state/AppState.h"

namespace sonora
{

AppState::AppState()
{
    loadMockProductSession();
}

void AppState::loadMockProductSession()
{
    hasTrack_ = true;
    loadedFilename_ = "NIGHT DRIVE.wav";
    backendStatus_ = BackendStatus::Ready;
    computeState_ = ComputeState::Complete;

    models::AudioAnalysis analysis;
    analysis.analyzerVersion = "SONORA_DSP_v0.1";
    analysis.bpm = 128.0f;
    analysis.durationSec = 192.4f;
    analysis.sampleRate = 44100;
    analysis.channels = 2;
    analysis.loudnessLufsApprox = -8.1f;
    analysis.dynamicRangeDb = 6.4f;
    analysis.stereoWidth = 0.78f;
    analysis.key.key = "A minor";
    analysis.key.confidence = 0.82f;
    analysis.key.method = "chroma_cqt";
    analysis.bands = { 0.18f, 0.34f, 0.27f, 0.14f, 0.07f };
    analysis_ = analysis;

    issues_ = {
        { "muddy_low_end", 0.73f, "low_end", "Excess energy around 80-120 Hz" },
        { "frequency_conflict", 0.58f, "low_end", "Kick and bass overlap" },
        { "narrow_stereo", 0.41f, "stereo_image", "Stereo width below recommended" },
    };

    insights_ = {
        { "Low frequency collision",
          "Kick and bass occupy the same 80-120 Hz region",
          "Reduce 120 Hz region on bass, keep kick transient",
          120.0f,
          1 },
        { "Masked mid presence",
          "Low-end energy is stealing headroom from vocals and leads",
          "Dynamic EQ around 220 Hz, then lift 2-4 kHz presence",
          220.0f,
          2 },
    };
}

void AppState::clearTrack()
{
    hasTrack_ = false;
    loadedFilename_.clear();
    analysis_.reset();
    issues_.clear();
    insights_.clear();
    backendStatus_ = BackendStatus::Connected;
    computeState_ = ComputeState::Ready;
}

void AppState::toggleMockTrack()
{
    if (hasTrack_)
        clearTrack();
    else
        loadMockProductSession();
}

} // namespace sonora
