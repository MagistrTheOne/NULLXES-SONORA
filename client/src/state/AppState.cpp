#include "state/AppState.h"

#include "backend/Dto.h"

#include <thread>

namespace sonora
{

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

void AppState::setTab(WorkspaceTab tab)
{
    tab_ = tab;
    notify();
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
    lastError_.clear();
    analysisState_ = AnalysisState::Empty;
    notify();
}

void AppState::pingHealth()
{
    runAsync([this] {
        const bool ok = api_.health();
        auto profileJson = ok ? api_.getProfile() : juce::var();
        applyOnMessage([this, ok, profileJson] {
            backendOnline_ = ok;
            if (!ok)
                lastError_ = api_.lastError();
            else if (analysisState_ == AnalysisState::Empty)
                lastError_.clear();
            if (ok && !profileJson.isVoid())
                profile_ = dto::parseProfile(profileJson);
            notify();
        });
    });
}

void AppState::analyzeFile(const juce::File& file)
{
    hasTrack_ = true;
    loadedFilename_ = file.getFileName().toStdString();
    analysis_.reset();
    issues_.clear();
    insights_.clear();
    harmony_.reset();
    eqProfile_.reset();
    lastError_.clear();
    analyzeProgress_ = 0.08f;
    analysisState_ = AnalysisState::Loading;
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
                lastError_ = api_.lastError();
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
                lastError_ = "Analyze response missing audio_id";
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
                        analysisState_ = AnalysisState::Complete;
                        analyzeProgress_ = 1.0f;
                        lastError_.clear();
                        notify();
                        requestEngineeringReport();
                    }
                    else
                    {
                        analysisState_ = AnalysisState::Failed;
                        lastError_ = "Analysis payload incomplete";
                        notify();
                    }
                });
                return;
            }

            if (status == "failed")
            {
                applyOnMessage([this, detail] {
                    analysisState_ = AnalysisState::Failed;
                    lastError_ = dto::parseError(detail);
                    if (lastError_.isEmpty())
                        lastError_ = "Analysis failed";
                    notify();
                });
                return;
            }

            juce::Thread::sleep(800);
        }

        applyOnMessage([this] {
            analysisState_ = AnalysisState::Failed;
            lastError_ = "Analysis timed out";
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
                lastError_ = api_.lastError();
            }
            else
            {
                insights_ = dto::parseInsights(json);
                if (!issues_.empty() && !insights_.empty())
                    insights_.front().confidence = issues_.front().severity;
                lastError_.clear();
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
                lastError_ = api_.lastError();
            else
            {
                harmony_ = dto::parseHarmony(json);
                lastError_.clear();
            }
            notify();
        });
    });
}

void AppState::createEqProfile()
{
    models::EqProfile profile;
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
