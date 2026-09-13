#pragma once

#include "backend/Fault.h"
#include "engine/Engine.h"
#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"
#include "soni/SoniTypes.h"
#include "ui/copy/HumanCopy.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sonora
{

class AudioPlayer;

enum class AnalysisState
{
    Empty,
    Loading,
    Analyzing,
    Complete,
    Failed
};

enum class WorkspaceTab
{
    Listen,
    Understand,
    Create,
    Soni
};

enum class CanvasNode
{
    Track,
    Understand,
    Improve,
    Create,
    Export
};

enum class UiMode
{
    Simple,
    Advanced
};

enum class ListenPhase
{
    Waiting,
    Listening,
    Mapping,
    Understood
};

struct LiveMixFlag
{
    juce::String name;
    int dir = 0;
};

class AppState : public juce::ChangeBroadcaster
{
public:
    AppState();
    ~AppState() override;

    AnalysisState analysisState() const { return analysisState_; }
    bool backendOnline() const { return true; }
    const Fault& fault() const { return fault_; }
    const juce::String& lastError() const { return fault_.reason; }
    float analyzeProgress() const { return analyzeProgress_; }

    bool hasTrack() const { return hasTrack_; }
    const std::string& loadedFilename() const { return loadedFilename_; }
    const std::optional<models::AudioAnalysis>& analysis() const { return analysis_; }
    const std::optional<models::AudioAnalysis>& previousAnalysis() const { return previousAnalysis_; }
    const std::vector<models::Issue>& issues() const { return issues_; }
    const std::vector<models::Insight>& insights() const { return insights_; }
    const std::optional<models::Harmony>& harmony() const { return harmony_; }
    const std::optional<models::MidiClip>& bassClip() const { return bassClip_; }
    const std::optional<models::MidiClip>& padClip() const { return padClip_; }
    const std::optional<models::DropPlan>& dropPlan() const { return dropPlan_; }
    const std::optional<models::EqProfile>& eqProfile() const { return eqProfile_; }
    const std::optional<models::AssistAdvice>& assistAdvice() const { return assistAdvice_; }
    const std::optional<models::ReferenceReport>& reference() const { return reference_; }
    bool referenceBusy() const { return referenceBusy_; }
    const models::SessionProfile& profile() const { return profile_; }
    const juce::String& audioId() const { return audioId_; }
    const juce::String& analysisId() const { return analysisId_; }
    WorkspaceTab tab() const { return tab_; }
    CanvasNode selectedNode() const { return selectedNode_; }
    UiMode uiMode() const { return uiMode_; }
    bool assistArmed() const { return labOpen_; }
    bool labOpen() const { return labOpen_; }
    bool referenceOpen() const { return referenceOpen_; }
    bool canCapture() const { return dawHost_ || (bool) captureStart_; }
    bool isListening() const { return listening_ || hostPlaying_; }
    bool busy() const { return inflight_.load() > 0; }
    bool dawHost() const { return dawHost_; }
    bool hostPlaying() const { return hostPlaying_; }
    double hostBpm() const { return hostBpm_; }
    double hostSeconds() const { return hostSeconds_; }
    int hostBar() const { return hostBar_; }
    float liveEnergy() const { return liveEnergy_; }
    float livePeak() const { return livePeak_; }
    float liveStereo() const { return liveStereo_; }
    juce::String nowSection() const;
    juce::String barLabel() const;
    bool soniMeetOpen() const { return soniMeetOpen_; }
    void setDawHost(bool enabled);
    void updateTransport(bool playing, double bpm, double ppq, double seconds);
    void updateLiveMeters(float energy, float peak, float stereo);
    void updateListenFill(float ratio);
    void analyzeLive(juce::AudioBuffer<float> buffer, double sampleRate, const juce::String& name);
    void dismissSoniMeet();
    bool trackUnderstood() const;
    ListenPhase listenPhase() const;
    float listenProgress() const;
    juce::String listenHeadline() const;
    juce::String listenHint() const;
    juce::String structureLine() const;
    juce::String mixLine() const;
    std::vector<juce::String> sonoraFound() const;
    std::vector<LiveMixFlag> liveMixFlags() const;
    juce::String toSessionJson() const;
    bool applySessionJson(const juce::String& json);
    void persistSession() const;
    void restoreSession();
    const models::TrackDna* dna() const;

    juce::String bpmLabel() const;
    juce::String keyLabel() const;
    juce::String sampleRateLabel() const;
    juce::String channelLabel() const;
    juce::String bitDepthLabel() const;
    juce::String durationLabel() const;
    juce::String issueCountLabel() const;
    juce::String objectCountLabel() const;
    juce::String styleLabel() const;
    juce::String energyLabel() const;
    float energyNow() const;
    std::vector<copy::IdentityAxis> sonicIdentity() const;
    std::vector<juce::String> profileLines() const;
    std::vector<juce::String> moodLabels() const;
    int healthScore() const;
    juce::String healthVerdict() const;
    copy::Delta mixDelta() const;
    copy::Finding assistFinding() const;
    std::vector<models::AssistOption> assistOptions() const;
    std::vector<copy::MixRow> mixRows() const;

    bool canPlay() const;
    bool isPlaying() const;
    float playhead() const;
    juce::String playheadLabel() const;
    float playheadSeconds() const;
    void togglePlayback();
    void seekPlayhead(float amount);

    void setTab(WorkspaceTab tab);
    void selectCanvasNode(CanvasNode node);
    void focusArrangement();
    void toggleUiMode();
    void armAssist();
    void disarmAssist();
    void openLab();
    void closeLab();
    void openReference();
    void closeReference();
    void armCapture(void* token);
    void* captureToken() const { return captureToken_; }
    bool handleKeyPress(const juce::KeyPress& key);
    void pingHealth();
    void setCaptureHooks(std::function<void()> start, std::function<void()> stop);
    void startListen();
    void stopListen();
    void failListen(const juce::String& reason);
    void analyzeFile(const juce::File& file);
    void analyzeBuffer(juce::AudioBuffer<float> buffer, double sampleRate, const juce::String& name);
    void requestEngineeringReport();
    void requestHarmony();
    void requestBass();
    void requestPad();
    void requestDrop();
    void requestArrangement();
    void requestAssist();
    void applyAssistOption(const juce::String& id);
    void compareReference(const juce::File& file);
    void createEqProfile();
    bool writeMidiFile(const juce::File& file, juce::String& error) const;
    void clearTrack();

    bool soniOpen() const { return soniOpen_; }
    bool soniMuted() const { return soniMuted_; }
    bool soniWelcomed() const { return soniWelcomed_; }
    const std::vector<soni::Message>& soniMessages() const { return soniMessages_; }
    void openSoni();
    void closeSoni();
    void toggleSoni();
    void setSoniMuted(bool muted);
    void ensureSoniWelcome();
    void sendSoniChat(const juce::String& text);
    soni::Context soniContext() const;

private:
    void notify();
    void runAsync(std::function<void()> work);
    void applyOnMessage(std::function<void()> fn);
    void resetGenerated();
    void applyResult(engine::Result result, const juce::String& name);
    void pushSoni(const juce::String& text);

    std::unique_ptr<AudioPlayer> player_;
    std::function<void()> captureStart_;
    std::function<void()> captureStop_;
    std::atomic<bool> alive_ { true };
    std::atomic<int> inflight_ { 0 };
    AnalysisState analysisState_ { AnalysisState::Empty };
    bool hasTrack_ = false;
    bool listening_ = false;
    float analyzeProgress_ = 0.0f;
    Fault fault_;
    juce::String audioId_;
    juce::String analysisId_;
    std::string loadedFilename_;
    std::optional<models::AudioAnalysis> analysis_;
    std::optional<models::AudioAnalysis> previousAnalysis_;
    std::vector<models::Issue> issues_;
    std::vector<models::Insight> insights_;
    std::optional<models::Harmony> harmony_;
    std::optional<models::MidiClip> bassClip_;
    std::optional<models::MidiClip> padClip_;
    std::optional<models::DropPlan> dropPlan_;
    std::optional<models::EqProfile> eqProfile_;
    std::optional<models::AssistAdvice> assistAdvice_;
    std::optional<models::ReferenceReport> reference_;
    bool referenceBusy_ = false;
    models::SessionProfile profile_;
    WorkspaceTab tab_ { WorkspaceTab::Listen };
    CanvasNode selectedNode_ { CanvasNode::Track };
    UiMode uiMode_ { UiMode::Simple };
    bool labOpen_ = false;
    bool referenceOpen_ = false;
    bool soniOpen_ = true;
    bool soniMuted_ = true;
    bool soniWelcomed_ = true;
    bool soniMeetOpen_ = false;
    bool dawHost_ = false;
    bool hostPlaying_ = false;
    double hostBpm_ = 0.0;
    double hostPpq_ = 0.0;
    double hostSeconds_ = 0.0;
    int hostBar_ = 0;
    float liveEnergy_ = 0.0f;
    float livePeak_ = 0.0f;
    float liveStereo_ = 0.0f;
    float listenFill_ = 0.0f;
    double heardSec_ = 0.0;
    bool restoring_ = false;
    bool metPersisted_ = false;
    juce::int64 lastLiveSoniMs_ = 0;
    std::vector<soni::Message> soniMessages_;
    void* captureToken_ = nullptr;

    JUCE_DECLARE_WEAK_REFERENCEABLE(AppState)
};

} // namespace sonora
