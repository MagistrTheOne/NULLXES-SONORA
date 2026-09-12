#pragma once

#include "backend/ApiClient.h"
#include "backend/Fault.h"
#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"

#include <juce_events/juce_events.h>

#include <atomic>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sonora
{

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
    Overview,
    Dna,
    Structure,
    Spectrum,
    Harmony,
    Generate,
    Mix,
    Master
};

enum class CanvasNode
{
    Input,
    Dna,
    Structure,
    Mix,
    Harmony,
    Export
};

class AppState : public juce::ChangeBroadcaster
{
public:
    AppState();
    ~AppState() override;

    AnalysisState analysisState() const { return analysisState_; }
    bool backendOnline() const { return backendOnline_; }
    const Fault& fault() const { return fault_; }
    const juce::String& lastError() const { return fault_.reason; }
    float analyzeProgress() const { return analyzeProgress_; }

    bool hasTrack() const { return hasTrack_; }
    const std::string& loadedFilename() const { return loadedFilename_; }
    const std::optional<models::AudioAnalysis>& analysis() const { return analysis_; }
    const std::vector<models::Issue>& issues() const { return issues_; }
    const std::vector<models::Insight>& insights() const { return insights_; }
    const std::optional<models::Harmony>& harmony() const { return harmony_; }
    const std::optional<models::EqProfile>& eqProfile() const { return eqProfile_; }
    const models::SessionProfile& profile() const { return profile_; }
    const juce::String& audioId() const { return audioId_; }
    const juce::String& analysisId() const { return analysisId_; }
    WorkspaceTab tab() const { return tab_; }
    CanvasNode selectedNode() const { return selectedNode_; }
    const models::TrackDna* dna() const;

    juce::String bpmLabel() const;
    juce::String keyLabel() const;
    juce::String sampleRateLabel() const;
    juce::String channelLabel() const;
    juce::String bitDepthLabel() const;
    juce::String issueCountLabel() const;
    std::vector<juce::String> profileLines() const;

    void setTab(WorkspaceTab tab);
    void selectCanvasNode(CanvasNode node);
    void focusArrangement();
    void pingHealth();
    void analyzeFile(const juce::File& file);
    void requestEngineeringReport();
    void requestHarmony();
    void createEqProfile();
    void clearTrack();

private:
    void notify();
    void runAsync(std::function<void()> work);
    void applyOnMessage(std::function<void()> fn);

    ApiClient api_;
    std::atomic<bool> alive_ { true };
    std::atomic<int> inflight_ { 0 };
    AnalysisState analysisState_ { AnalysisState::Empty };
    bool backendOnline_ = false;
    bool hasTrack_ = false;
    float analyzeProgress_ = 0.0f;
    Fault fault_;
    juce::String audioId_;
    juce::String analysisId_;
    std::string loadedFilename_;
    std::optional<models::AudioAnalysis> analysis_;
    std::vector<models::Issue> issues_;
    std::vector<models::Insight> insights_;
    std::optional<models::Harmony> harmony_;
    std::optional<models::EqProfile> eqProfile_;
    models::SessionProfile profile_;
    WorkspaceTab tab_ { WorkspaceTab::Overview };
    CanvasNode selectedNode_ { CanvasNode::Input };

    JUCE_DECLARE_WEAK_REFERENCEABLE(AppState)
};

} // namespace sonora
