#include "state/AppState.h"

#include "audio/AudioPlayer.h"
#include "engine/Engine.h"

#include <thread>

namespace sonora
{
namespace
{
models::Insight insightFromIssue(const models::Issue& issue)
{
    models::Insight row;
    row.issue = issue.type;
    row.reason = copy::issuePhrase(issue).toStdString();
    row.confidence = issue.severity;
    row.operation = "EQ";
    if (issue.type == "clipping")
        row.action = "Reduce peak gain";
    else if (issue.type == "narrow_stereo")
        row.action = "Widen the image";
    else if (issue.type == "low_dynamic_range")
        row.action = "Restore dynamics";
    else
    {
        row.action = "Fix vocal space";
        row.frequencyHz = issue.type == "frequency_conflict" ? 250.0f : 120.0f;
    }
    return row;
}
} // namespace

AppState::AppState()
    : player_(std::make_unique<AudioPlayer>())
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

juce::String AppState::durationLabel() const
{
    if (!analysis_)
        return "--:--";
    return copy::formatTime(analysis_->durationSec);
}

juce::String AppState::issueCountLabel() const
{
    return juce::String((int) issues_.size()) + (issues_.size() == 1 ? " ISSUE" : " ISSUES");
}

juce::String AppState::objectCountLabel() const
{
    int count = 0;
    if (const auto* model = dna())
        count += (int) model->objects.size();
    if (harmony_)
        ++count;
    if (bassClip_)
        ++count;
    if (padClip_)
        ++count;
    if (dropPlan_)
        ++count;
    if (eqProfile_)
        ++count;
    if (reference_)
        ++count;
    return juce::String(count) + (count == 1 ? " OBJECT" : " OBJECTS");
}

juce::String AppState::styleLabel() const
{
    return copy::styleLine(dna(), profileLines());
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
    if (const auto* model = dna())
    {
        for (const auto& tag : model->genreProfile)
            if (!tag.empty())
                lines.push_back(juce::String(tag));
    }
    if (lines.empty())
    {
        lines.emplace_back("Deep House");
        lines.emplace_back("Slap House");
    }
    std::vector<juce::String> unique;
    for (const auto& line : lines)
    {
        bool seen = false;
        for (const auto& existing : unique)
            if (existing.equalsIgnoreCase(line))
                seen = true;
        if (!seen)
            unique.push_back(line);
    }
    if (unique.size() > 3)
        unique.resize(3);
    return unique;
}

std::vector<juce::String> AppState::moodLabels() const
{
    if (!analysis_)
        return { "---" };
    return copy::moodTags(*analysis_);
}

int AppState::healthScore() const
{
    if (!analysis_)
        return 0;
    return copy::healthScore(*analysis_, issues_);
}

juce::String AppState::healthVerdict() const
{
    return copy::healthVerdict(healthScore());
}

copy::Delta AppState::mixDelta() const
{
    if (!previousAnalysis_ || !analysis_)
        return {};
    return copy::mixDelta(*previousAnalysis_, *analysis_);
}

copy::Finding AppState::assistFinding() const
{
    if (assistAdvice_ && !assistAdvice_->headline.empty())
        return { juce::String(assistAdvice_->headline), juce::String(assistAdvice_->detail) };
    if (!analysis_)
        return { "Load a track.", "SONORA will listen, then we can work." };
    return copy::assistFinding(*analysis_, issues_);
}

std::vector<models::AssistOption> AppState::assistOptions() const
{
    if (assistAdvice_ && !assistAdvice_->options.empty())
        return assistAdvice_->options;
    return {
        { "strengthen_drop", "Strengthen drop" },
        { "fix_vocal_space", "Fix vocal space" },
        { "create_bass", "Create bass" },
        { "compare_reference", "Compare reference" },
    };
}

bool AppState::canPlay() const
{
    return player_ != nullptr && player_->isReady();
}

bool AppState::isPlaying() const
{
    return player_ != nullptr && player_->isPlaying();
}

float AppState::playhead() const
{
    return player_ != nullptr ? player_->positionNormalized() : 0.0f;
}

juce::String AppState::playheadLabel() const
{
    if (player_ == nullptr || !player_->isReady())
        return durationLabel();
    return copy::formatTime((float) player_->positionSeconds()) + " / " + durationLabel();
}

void AppState::togglePlayback()
{
    if (player_ == nullptr)
        return;
    player_->toggle();
    notify();
}

void AppState::seekPlayhead(float amount)
{
    if (player_ == nullptr)
        return;
    player_->seekNormalized(amount);
    notify();
}

std::vector<copy::MixRow> AppState::mixRows() const
{
    if (!analysis_)
        return {};
    return copy::mixRows(*analysis_, issues_);
}

const models::TrackDna* AppState::dna() const
{
    if (!analysis_ || !analysis_->hasDna)
        return nullptr;
    return &analysis_->dna;
}

void AppState::setTab(WorkspaceTab tab)
{
    tab_ = tab;
    switch (tab)
    {
        case WorkspaceTab::Track: selectedNode_ = CanvasNode::Track; break;
        case WorkspaceTab::Mix: selectedNode_ = CanvasNode::Improve; break;
        case WorkspaceTab::Arrangement: selectedNode_ = CanvasNode::Understand; break;
        case WorkspaceTab::Create: selectedNode_ = CanvasNode::Create; break;
        case WorkspaceTab::Reference: selectedNode_ = CanvasNode::Understand; break;
        default: break;
    }
    notify();
}

void AppState::selectCanvasNode(CanvasNode node)
{
    selectedNode_ = node;
    switch (node)
    {
        case CanvasNode::Track: tab_ = WorkspaceTab::Track; break;
        case CanvasNode::Understand: tab_ = WorkspaceTab::Track; armAssist(); return;
        case CanvasNode::Improve: tab_ = WorkspaceTab::Mix; break;
        case CanvasNode::Create: tab_ = WorkspaceTab::Create; break;
        case CanvasNode::Export: tab_ = WorkspaceTab::Create; break;
    }
    notify();
}

void AppState::focusArrangement()
{
    setTab(WorkspaceTab::Arrangement);
}

void AppState::toggleUiMode()
{
    uiMode_ = uiMode_ == UiMode::Simple ? UiMode::Advanced : UiMode::Simple;
    notify();
}

void AppState::armAssist()
{
    assistArmed_ = true;
    tab_ = WorkspaceTab::Track;
    selectedNode_ = CanvasNode::Understand;
    notify();
}

void AppState::disarmAssist()
{
    if (!assistArmed_)
        return;
    assistArmed_ = false;
    notify();
}

bool AppState::handleKeyPress(const juce::KeyPress& key)
{
    if (key == juce::KeyPress('l', juce::ModifierKeys::ctrlModifier, 0)
        || key == juce::KeyPress('L', juce::ModifierKeys::ctrlModifier, 0))
    {
        armAssist();
        return true;
    }
    if (key == juce::KeyPress::escapeKey && assistArmed_)
    {
        disarmAssist();
        return true;
    }
    if (key == juce::KeyPress::spaceKey && canPlay())
    {
        togglePlayback();
        return true;
    }
    return false;
}

void AppState::resetGenerated()
{
    insights_.clear();
    harmony_.reset();
    bassClip_.reset();
    padClip_.reset();
    dropPlan_.reset();
    eqProfile_.reset();
    assistAdvice_.reset();
    reference_.reset();
    referenceBusy_ = false;
}

void AppState::clearTrack()
{
    if (player_ != nullptr)
        player_->unload();
    listening_ = false;
    hasTrack_ = false;
    loadedFilename_.clear();
    analysis_.reset();
    issues_.clear();
    resetGenerated();
    audioId_.clear();
    analysisId_.clear();
    analyzeProgress_ = 0.0f;
    fault_ = {};
    analysisState_ = AnalysisState::Empty;
    selectedNode_ = CanvasNode::Track;
    tab_ = WorkspaceTab::Track;
    assistArmed_ = false;
    notify();
}

void AppState::pingHealth()
{
    fault_ = {};
    notify();
}

void AppState::setCaptureHooks(std::function<void()> start, std::function<void()> stop)
{
    captureStart_ = std::move(start);
    captureStop_ = std::move(stop);
}

void AppState::startListen()
{
    if (captureStart_ == nullptr || listening_)
        return;
    listening_ = true;
    hasTrack_ = true;
    loadedFilename_ = "DAW capture";
    analysisState_ = AnalysisState::Analyzing;
    analyzeProgress_ = 0.12f;
    fault_ = {};
    notify();
    captureStart_();
}

void AppState::stopListen()
{
    if (!listening_)
        return;
    listening_ = false;
    if (captureStop_ != nullptr)
        captureStop_();
}

void AppState::failListen(const juce::String& reason)
{
    listening_ = false;
    analysisState_ = AnalysisState::Failed;
    analyzeProgress_ = 0.0f;
    fault_ = { "LISTEN FAILED", 0, reason };
    notify();
}

void AppState::applyResult(engine::Result result, const juce::String& name)
{
    if (!result.ok())
    {
        analysisState_ = AnalysisState::Failed;
        fault_ = { "ANALYSIS FAILED", 0, result.error };
        notify();
        return;
    }
    loadedFilename_ = name.toStdString();
    audioId_ = "local";
    analysisId_ = "local";
    analysis_ = std::move(result.analysis);
    issues_ = std::move(result.issues);
    insights_.clear();
    for (const auto& issue : issues_)
        insights_.push_back(insightFromIssue(issue));
    assistAdvice_ = engine::makeAssist(*analysis_, issues_);
    analysisState_ = AnalysisState::Complete;
    analyzeProgress_ = 1.0f;
    selectedNode_ = CanvasNode::Understand;
    tab_ = WorkspaceTab::Track;
    fault_ = {};
    notify();
}

void AppState::analyzeFile(const juce::File& file)
{
    if (analysis_)
        previousAnalysis_ = *analysis_;
    hasTrack_ = true;
    loadedFilename_ = file.getFileName().toStdString();
    if (player_ != nullptr)
        player_->load(file);
    analysis_.reset();
    issues_.clear();
    resetGenerated();
    fault_ = {};
    analyzeProgress_ = 0.08f;
    analysisState_ = AnalysisState::Loading;
    selectedNode_ = CanvasNode::Track;
    tab_ = WorkspaceTab::Track;
    assistArmed_ = false;
    notify();

    runAsync([this, file] {
        applyOnMessage([this] {
            analysisState_ = AnalysisState::Analyzing;
            analyzeProgress_ = 0.35f;
            notify();
        });
        juce::AudioBuffer<float> buffer;
        double sr = 0.0;
        juce::String error;
        if (!engine::loadFile(file, buffer, sr, error))
        {
            applyOnMessage([this, error] {
                analysisState_ = AnalysisState::Failed;
                fault_ = { "LOAD FAILED", 0, error };
                notify();
            });
            return;
        }
        auto result = engine::analyze(buffer, sr);
        applyOnMessage([this, result = std::move(result), name = file.getFileName()]() mutable {
            applyResult(std::move(result), name);
        });
    });
}

void AppState::analyzeBuffer(juce::AudioBuffer<float> buffer, double sampleRate, const juce::String& name)
{
    if (analysis_)
        previousAnalysis_ = *analysis_;
    hasTrack_ = true;
    loadedFilename_ = name.toStdString();
    analysis_.reset();
    issues_.clear();
    resetGenerated();
    listening_ = false;
    fault_ = {};
    analyzeProgress_ = 0.2f;
    analysisState_ = AnalysisState::Analyzing;
    selectedNode_ = CanvasNode::Track;
    tab_ = WorkspaceTab::Track;
    notify();

    runAsync([this, buffer = std::move(buffer), sampleRate, name] {
        auto result = engine::analyze(buffer, sampleRate);
        applyOnMessage([this, result = std::move(result), name]() mutable {
            applyResult(std::move(result), name);
        });
    });
}

void AppState::requestEngineeringReport()
{
    requestAssist();
}

void AppState::requestHarmony()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    harmony_ = engine::makeHarmony(*analysis_);
    fault_ = {};
    notify();
}

void AppState::requestBass()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    bassClip_ = engine::makeBass(*analysis_);
    fault_ = {};
    notify();
}

void AppState::requestPad()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    padClip_ = engine::makePad(*analysis_);
    fault_ = {};
    notify();
}

void AppState::requestDrop()
{
    if (!analysis_)
        return;
    dropPlan_ = engine::makeDrop(*analysis_);
    models::EqProfile profile;
    profile.operation = "EQ";
    profile.target = dropPlan_->sectionName.empty() ? "drop" : dropPlan_->sectionName;
    profile.frequencyHz = dropPlan_->frequency;
    profile.gainDb = dropPlan_->gain;
    eqProfile_ = profile;
    fault_ = {};
    notify();
}

void AppState::requestAssist()
{
    if (!analysis_)
        return;
    assistAdvice_ = engine::makeAssist(*analysis_, issues_);
    fault_ = {};
    notify();
}

void AppState::applyAssistOption(const juce::String& id)
{
    if (id == "strengthen_drop")
        requestDrop();
    else if (id == "create_bass" || id == "bass")
        requestBass();
    else if (id == "create_pad" || id == "pad")
        requestPad();
    else if (id == "fix_vocal_space")
        createEqProfile();
    else if (id == "compare_reference")
        setTab(WorkspaceTab::Reference);
    else if (id == "create_chords" || id == "harmony")
        requestHarmony();
}

void AppState::compareReference(const juce::File& file)
{
    if (!analysis_)
        return;
    referenceBusy_ = true;
    notify();
    runAsync([this, file] {
        juce::AudioBuffer<float> buffer;
        double sr = 0.0;
        juce::String error;
        engine::Result ref;
        if (engine::loadFile(file, buffer, sr, error))
            ref = engine::analyze(buffer, sr);
        else
            ref.error = error;
        applyOnMessage([this, ref = std::move(ref), name = file.getFileName()]() mutable {
            referenceBusy_ = false;
            if (!ref.ok() || !analysis_)
                fault_ = { "REFERENCE FAILED", 0, ref.error.isEmpty() ? "cannot compare" : ref.error };
            else
            {
                reference_ = engine::compare(*analysis_, ref.analysis, juce::String(loadedFilename_), name);
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
                setTab(WorkspaceTab::Mix);
                return;
            }
        }
    }
    if (!insights_.empty())
    {
        const auto& insight = insights_.front();
        profile.operation = insight.operation.empty() ? "EQ" : insight.operation;
        profile.target = insight.issue.empty() ? insight.action : insight.issue;
        if (insight.frequencyHz.has_value())
            profile.frequencyHz = *insight.frequencyHz;
        else
            profile.frequencyHz = 120.0f;
    }
    else if (!issues_.empty())
    {
        profile.target = issues_.front().type;
        profile.operation = "EQ";
        profile.frequencyHz = 120.0f;
    }
    eqProfile_ = profile;
    notify();
}

} // namespace sonora
