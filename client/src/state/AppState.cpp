#include "state/AppState.h"

#include "backend/ClientLog.h"
#include "backend/Dto.h"

#include <thread>

namespace sonora
{
namespace
{
models::Insight insightFromIssue(const models::Issue& issue)
{
    models::Insight row;
    row.issue = issue.type;
    row.reason = issue.detail.empty() ? issue.type : issue.detail;
    row.confidence = issue.severity;
    row.operation = "Dynamic EQ";
    if (issue.type == "clipping")
        row.action = "Reduce peak gain";
    else if (issue.type == "narrow_stereo")
        row.action = "Widen mid/side";
    else if (issue.type == "low_dynamic_range")
        row.action = "Restore dynamics";
    else
    {
        row.action = "Create EQ profile";
        row.frequencyHz = issue.type == "frequency_conflict" ? 250.0f : 120.0f;
    }
    return row;
}
} // namespace

AppState::AppState()
{
    pingHealth();
}

AppState::~AppState()
{
    alive_ = false;
    const auto deadline = juce::Time::getMillisecondCounter() + 8000;
    while (inflight_ > 0 && juce::Time::getMillisecondCounter() < deadline)
        juce::Thread::sleep(20);
}

void AppState::notify()
{
    if (alive_)
        sendChangeMessage();
}

void AppState::runAsync(std::function<void()> work)
{
    ++inflight_;
    std::thread([this, fn = std::move(work)] {
        fn();
        --inflight_;
    }).detach();
}

void AppState::applyOnMessage(std::function<void()> fn)
{
    juce::WeakReference<AppState> safe(this);
    juce::MessageManager::callAsync([safe, fn = std::move(fn)] {
        if (safe != nullptr)
            fn();
    });
}

juce::String AppState::bpmLabel() const
{
    if (!analysis_)
        return "---";
    return juce::String(juce::roundToInt(analysis_->bpm));
}

juce::String AppState::keyLabel() const
{
    if (!analysis_ || !analysis_->key.key.has_value())
        return "---";
    return juce::String(*analysis_->key.key);
}

juce::String AppState::sampleRateLabel() const
{
    if (!analysis_ || analysis_->sampleRate <= 0)
        return "Waiting";
    return juce::String(analysis_->sampleRate / 1000.0, 1) + " kHz";
}

juce::String AppState::channelLabel() const
{
    if (!analysis_)
        return "Waiting";
    if (analysis_->channels == 2)
        return "Stereo";
    if (analysis_->channels == 1)
        return "Mono";
    return juce::String(analysis_->channels) + " ch";
}

juce::String AppState::bitDepthLabel() const
{
    return analysis_ ? "24 bit" : "Waiting";
}

juce::String AppState::issueCountLabel() const
{
    return juce::String((int) issues_.size()) + (issues_.size() == 1 ? " ISSUE" : " ISSUES");
}

std::vector<juce::String> AppState::profileLines() const
{
    std::vector<juce::String> lines;
    for (const auto& item : profile_.style)
        if (!item.empty())
            lines.push_back(juce::String(item));
    for (const auto& item : profile_.genres)
        if (!item.empty())
            lines.push_back(juce::String(item));
    if (lines.empty())
    {
        lines.emplace_back("Deep House");
        lines.emplace_back("Slap House");
    }
    if (lines.size() > 3)
        lines.resize(3);
    return lines;
}

const models::TrackDna* AppState::dna() const
{
    if (!analysis_ || !analysis_->hasDna)
        return nullptr;
    return &analysis_->dna;
}

void AppState::setTab(WorkspaceTab tab)
{
    if (tab == WorkspaceTab::Mix || tab == WorkspaceTab::Master)
        return;
    tab_ = tab;
    switch (tab)
    {
        case WorkspaceTab::Overview: selectedNode_ = CanvasNode::Input; break;
        case WorkspaceTab::Dna: selectedNode_ = CanvasNode::Dna; break;
        case WorkspaceTab::Structure: selectedNode_ = CanvasNode::Structure; break;
        case WorkspaceTab::Harmony: selectedNode_ = CanvasNode::Harmony; break;
        case WorkspaceTab::Generate: selectedNode_ = CanvasNode::Export; break;
        default: break;
    }
    notify();
}

void AppState::selectCanvasNode(CanvasNode node)
{
    selectedNode_ = node;
    switch (node)
    {
        case CanvasNode::Input: tab_ = WorkspaceTab::Overview; break;
        case CanvasNode::Dna: tab_ = WorkspaceTab::Dna; break;
        case CanvasNode::Structure: tab_ = WorkspaceTab::Structure; break;
        case CanvasNode::Mix: tab_ = WorkspaceTab::Dna; break;
        case CanvasNode::Harmony: tab_ = WorkspaceTab::Harmony; break;
        case CanvasNode::Export: tab_ = WorkspaceTab::Generate; break;
    }
    notify();
}

void AppState::focusArrangement()
{
    selectCanvasNode(CanvasNode::Structure);
}

void AppState::clearTrack()
{
    hasTrack_ = false;
    loadedFilename_.clear();
    analysis_.reset();
    issues_.clear();
    insights_.clear();
    harmony_.reset();
    eqProfile_.reset();
    audioId_.clear();
    analysisId_.clear();
    analyzeProgress_ = 0.0f;
    fault_ = {};
    analysisState_ = AnalysisState::Empty;
    selectedNode_ = CanvasNode::Input;
    tab_ = WorkspaceTab::Overview;
    notify();
}

void AppState::pingHealth()
{
    runAsync([this] {
        const bool ok = api_.health();
        auto profileJson = ok ? api_.getProfile() : juce::var();
        applyOnMessage([this, ok, profileJson] {
            backendOnline_ = ok;
            if (!ok && analysisState_ == AnalysisState::Empty)
                fault_ = api_.lastFault();
            else if (ok && analysisState_ == AnalysisState::Empty)
                fault_ = {};
            if (ok && !profileJson.isVoid())
                profile_ = dto::parseProfile(profileJson);
            notify();
        });
    });
}

void AppState::analyzeFile(const juce::File& file)
{
    clientLog("AppState analyzeFile " + file.getFullPathName());
    hasTrack_ = true;
    loadedFilename_ = file.getFileName().toStdString();
    analysis_.reset();
    issues_.clear();
    insights_.clear();
    harmony_.reset();
    eqProfile_.reset();
    fault_ = {};
    analyzeProgress_ = 0.08f;
    analysisState_ = AnalysisState::Loading;
    selectedNode_ = CanvasNode::Input;
    tab_ = WorkspaceTab::Overview;
    selectedNode_ = CanvasNode::Dna;
    tab_ = WorkspaceTab::Overview;
    selectedNode_ = CanvasNode::Input;
    tab_ = WorkspaceTab::Overview;
    clientLog("AppState analyzeFile " + file.getFullPathName() + " bytes=" + juce::String(file.getSize()));
    notify();

    runAsync([this, file] {
        applyOnMessage([this] {
            analysisState_ = AnalysisState::Analyzing;
            analyzeProgress_ = 0.18f;
            notify();
        });

        auto enqueued = api_.analyzeFile(file);
        if (enqueued.isVoid())
        {
            applyOnMessage([this] {
                analysisState_ = AnalysisState::Failed;
                fault_ = api_.lastFault();
                if (fault_.empty())
                    fault_ = { "UPLOAD FAILED", 0, "unknown error" };
                notify();
            });
            return;
        }

        const auto audioId = dto::parseAudioId(enqueued);
        const auto analysisId = dto::parseAnalysisId(enqueued);
        if (audioId.isEmpty())
        {
            applyOnMessage([this] {
                analysisState_ = AnalysisState::Failed;
                fault_ = { "UPLOAD FAILED", 0, "analyze response missing audio_id" };
                notify();
            });
            return;
        }

        for (int i = 0; i < 90 && alive_; ++i)
        {
            auto detail = api_.getAudio(audioId);
            const auto status = dto::parseAnalysisStatus(detail);
            applyOnMessage([this, i] {
                analyzeProgress_ = juce::jmin(0.95f, 0.22f + (float) i * 0.016f);
                notify();
            });

            if (status == "completed")
            {
                models::AudioAnalysis parsed;
                std::vector<models::Issue> parsedIssues;
                const bool ok = dto::parseCompletedAnalysis(detail, parsed, parsedIssues);
                applyOnMessage([this, ok, parsed, parsedIssues, audioId, analysisId] {
                    audioId_ = audioId;
                    analysisId_ = analysisId;
                    if (ok)
                    {
                        analysis_ = parsed;
                        issues_ = parsedIssues;
                        insights_.clear();
                        for (const auto& issue : parsedIssues)
                            insights_.push_back(insightFromIssue(issue));
                        analysisState_ = AnalysisState::Complete;
                        analyzeProgress_ = 1.0f;
                        selectedNode_ = CanvasNode::Dna;
                        tab_ = WorkspaceTab::Dna;
                        fault_ = {};
                        notify();
                    }
                    else
                    {
                        analysisState_ = AnalysisState::Failed;
                        fault_ = { "ANALYSIS FAILED", 0, "payload incomplete" };
                        notify();
                    }
                });
                return;
            }

            if (status == "failed")
            {
                applyOnMessage([this, detail] {
                    analysisState_ = AnalysisState::Failed;
                    fault_ = { "ANALYSIS FAILED", 0, dto::parseError(detail) };
                    if (fault_.reason.isEmpty())
                        fault_.reason = "analysis failed";
                    notify();
                });
                return;
            }

            juce::Thread::sleep(800);
        }

        applyOnMessage([this] {
            analysisState_ = AnalysisState::Failed;
            fault_ = { "ANALYSIS FAILED", 0, "timed out" };
            notify();
        });
    });
}

void AppState::requestEngineeringReport()
{
    if (analysisId_.isEmpty())
        return;
    runAsync([this] {
        auto json = api_.generateReport(analysisId_);
        applyOnMessage([this, json] {
            if (json.isVoid())
            {
                fault_ = api_.lastFault();
            }
            else
            {
                insights_ = dto::parseInsights(json);
                if (!issues_.empty() && !insights_.empty())
                    insights_.front().confidence = issues_.front().severity;
                fault_ = {};
            }
            notify();
        });
    });
}

void AppState::requestHarmony()
{
    if (analysisId_.isEmpty())
        return;
    setTab(WorkspaceTab::Harmony);
    runAsync([this] {
        auto json = api_.generateHarmony(analysisId_);
        applyOnMessage([this, json] {
            if (json.isVoid())
                fault_ = api_.lastFault();
            else
            {
                harmony_ = dto::parseHarmony(json);
                fault_ = {};
            }
            notify();
        });
    });
}

void AppState::createEqProfile()
{
    models::EqProfile profile;
    if (const auto* model = dna())
    {
        for (const auto& object : model->objects)
        {
            if (object.type == "EQ_PROFILE")
            {
                profile.operation = "EQ";
                profile.target = object.input;
                profile.frequencyHz = object.frequency > 0.0f ? object.frequency : 120.0f;
                profile.gainDb = object.gain != 0.0f ? object.gain : -3.0f;
                eqProfile_ = profile;
                selectCanvasNode(CanvasNode::Mix);
                return;
            }
        }
    }
    if (!insights_.empty())
    {
        const auto& insight = insights_.front();
        profile.operation = insight.operation.empty() ? "Dynamic EQ" : insight.operation;
        profile.target = insight.issue.empty() ? insight.action : insight.issue;
        if (insight.frequencyHz.has_value())
            profile.frequencyHz = *insight.frequencyHz;
        else
            profile.frequencyHz = 120.0f;
    }
    else if (!issues_.empty())
    {
        profile.target = issues_.front().type;
        profile.operation = "Dynamic EQ";
        profile.frequencyHz = 120.0f;
    }
    eqProfile_ = profile;
    notify();
}

} // namespace sonora
