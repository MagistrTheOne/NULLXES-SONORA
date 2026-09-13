#include "state/AppState.h"

#include "audio/AudioPlayer.h"
#include "engine/Engine.h"
#include "session/SessionStore.h"
#include "soni/SoniBrain.h"

#include <cmath>
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
#ifndef SONORA_IS_PLUGIN
    restoreSession();
#endif
}

AppState::~AppState()
{
    persistSession();
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
    if (dawHost_ && hostBpm_ > 1.0)
        return juce::String(juce::roundToInt(hostBpm_));
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
    if (!analysis_)
        return "---";
    return copy::styleFromIdentity(copy::sonicIdentity(*analysis_, issues_));
}

juce::String AppState::energyLabel() const
{
    if (!analysis_)
        return "---";
    return juce::String(juce::roundToInt(energyNow() * 100.0f)) + "%";
}

float AppState::energyNow() const
{
    if (dawHost_ && (hostPlaying_ || liveEnergy_ > 0.0f))
        return juce::jlimit(0.0f, 1.0f, liveEnergy_ * 4.5f);
    const auto* model = dna();
    if (model == nullptr)
        return 0.0f;
    const auto& curve = !model->energyCurve.empty() ? model->energyCurve : model->energyPeaks;
    if (curve.empty())
        return model->energyMean;
    const float t = juce::jlimit(0.0f, 1.0f, playhead());
    const int i = juce::jlimit(0, (int) curve.size() - 1, juce::roundToInt(t * (float) (curve.size() - 1)));
    return juce::jlimit(0.0f, 1.0f, curve[(size_t) i]);
}

std::vector<copy::IdentityAxis> AppState::sonicIdentity() const
{
    if (!analysis_)
        return {};
    return copy::sonicIdentity(*analysis_, issues_);
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
        for (const auto& axis : sonicIdentity())
            if (axis.value >= 0.52f)
                lines.push_back(axis.name);
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
    return copy::labOptions();
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
    return copy::formatTime(playheadSeconds()) + " / " + durationLabel();
}

float AppState::playheadSeconds() const
{
    if (player_ != nullptr && player_->isReady())
        return (float) player_->positionSeconds();
    if (analysis_)
        return playhead() * analysis_->durationSec;
    return 0.0f;
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
    referenceOpen_ = false;
    labOpen_ = false;
    switch (tab)
    {
        case WorkspaceTab::Listen: selectedNode_ = CanvasNode::Track; break;
        case WorkspaceTab::Understand: selectedNode_ = CanvasNode::Understand; break;
        case WorkspaceTab::Create: selectedNode_ = CanvasNode::Create; break;
        case WorkspaceTab::Soni: selectedNode_ = CanvasNode::Understand; soniOpen_ = true; break;
    }
    notify();
}

void AppState::selectCanvasNode(CanvasNode node)
{
    selectedNode_ = node;
    switch (node)
    {
        case CanvasNode::Track: tab_ = WorkspaceTab::Listen; break;
        case CanvasNode::Understand: tab_ = WorkspaceTab::Understand; break;
        case CanvasNode::Improve: tab_ = WorkspaceTab::Understand; break;
        case CanvasNode::Create: tab_ = WorkspaceTab::Create; break;
        case CanvasNode::Export: tab_ = WorkspaceTab::Create; break;
    }
    notify();
}

void AppState::focusArrangement()
{
    requestArrangement();
}

void AppState::toggleUiMode()
{
    uiMode_ = uiMode_ == UiMode::Simple ? UiMode::Advanced : UiMode::Simple;
    notify();
}

void AppState::armAssist()
{
    openLab();
}

void AppState::disarmAssist()
{
    closeLab();
}

void AppState::openLab()
{
    labOpen_ = true;
    selectedNode_ = CanvasNode::Understand;
    notify();
}

void AppState::closeLab()
{
    if (!labOpen_)
        return;
    labOpen_ = false;
    notify();
}

void AppState::openReference()
{
    referenceOpen_ = true;
    labOpen_ = false;
    tab_ = WorkspaceTab::Understand;
    notify();
}

void AppState::closeReference()
{
    if (!referenceOpen_)
        return;
    referenceOpen_ = false;
    notify();
}

void AppState::armCapture(void* token)
{
    captureToken_ = token;
}

bool AppState::handleKeyPress(const juce::KeyPress& key)
{
    if (key == juce::KeyPress('l', juce::ModifierKeys::ctrlModifier, 0)
        || key == juce::KeyPress('L', juce::ModifierKeys::ctrlModifier, 0))
    {
        armAssist();
        return true;
    }
    if (key == juce::KeyPress('j', juce::ModifierKeys::ctrlModifier, 0)
        || key == juce::KeyPress('J', juce::ModifierKeys::ctrlModifier, 0))
    {
        toggleSoni();
        return true;
    }
    if (key == juce::KeyPress::escapeKey && (labOpen_ || referenceOpen_))
    {
        closeLab();
        closeReference();
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
    listenFill_ = 0.0f;
    heardSec_ = 0.0;
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
    tab_ = WorkspaceTab::Listen;
    labOpen_ = false;
    referenceOpen_ = false;
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
    pushSoni(soni::afterFail(reason));
    notify();
}

void AppState::applyResult(engine::Result result, const juce::String& name)
{
    if (!result.ok())
    {
        analysisState_ = AnalysisState::Failed;
        fault_ = { "ANALYSIS FAILED", 0, result.error };
        pushSoni(soni::afterFail(result.error));
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
    tab_ = WorkspaceTab::Listen;
    fault_ = {};
    pushSoni(dawHost_ ? soni::afterLive(soniContext()) : soni::afterListen(soniContext()));
    persistSession();
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
    tab_ = WorkspaceTab::Listen;
    labOpen_ = false;
    referenceOpen_ = false;
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
    tab_ = WorkspaceTab::Listen;
    labOpen_ = false;
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

void AppState::requestArrangement()
{
    if (!analysis_)
        return;
    closeLab();
    setTab(WorkspaceTab::Create);
}

void AppState::requestHarmony()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    harmony_ = engine::makeHarmony(*analysis_);
    fault_ = {};
    persistSession();
    notify();
}

void AppState::requestBass()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    bassClip_ = engine::makeBass(*analysis_);
    fault_ = {};
    persistSession();
    notify();
}

void AppState::requestPad()
{
    if (!analysis_)
        return;
    setTab(WorkspaceTab::Create);
    padClip_ = engine::makePad(*analysis_);
    fault_ = {};
    persistSession();
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
    persistSession();
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
        openReference();
    else if (id == "create_chords" || id == "harmony")
        requestHarmony();
    else if (id == "build_arrangement" || id == "create_arrangement")
        requestArrangement();
    closeLab();
}

bool AppState::writeMidiFile(const juce::File& file, juce::String& error) const
{
    return engine::writeMidiFile(file, harmony_, bassClip_, padClip_, error);
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
                referenceOpen_ = true;
                tab_ = WorkspaceTab::Understand;
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
                setTab(WorkspaceTab::Understand);
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
    persistSession();
    notify();
}

void AppState::setDawHost(bool enabled)
{
    dawHost_ = enabled;
}

void AppState::updateTransport(bool playing, double bpm, double ppq, double seconds)
{
    const bool started = playing && !hostPlaying_;
    const bool stopped = !playing && hostPlaying_;
    if (playing && hostPlaying_)
    {
        const double dt = seconds - hostSeconds_;
        heardSec_ += (dt > 0.0 && dt < 1.0) ? dt : 0.2;
    }
    hostPlaying_ = playing;
    hostBpm_ = bpm;
    hostPpq_ = ppq;
    hostSeconds_ = seconds;
    const int nextBar = ppq > 0.0 ? (int) std::floor(ppq / 4.0) + 1 : 0;
    const bool barChanged = nextBar != hostBar_;
    hostBar_ = nextBar;
    if (dawHost_ && started)
    {
        listening_ = true;
        hasTrack_ = true;
        if (loadedFilename_.empty())
            loadedFilename_ = "FL session";
        if (analysisState_ == AnalysisState::Empty)
        {
            analysisState_ = AnalysisState::Analyzing;
            analyzeProgress_ = 0.12f;
        }
        soniMeetOpen_ = false;
        metPersisted_ = true;
    }
    if (dawHost_ && !playing)
        listening_ = false;
    if (started || stopped || barChanged)
        notify();
}

void AppState::updateLiveMeters(float energy, float peak, float stereo)
{
    liveEnergy_ = energy;
    livePeak_ = peak;
    liveStereo_ = stereo;
}

void AppState::updateListenFill(float ratio)
{
    listenFill_ = juce::jlimit(0.0f, 1.0f, ratio);
    if (analysisState_ != AnalysisState::Complete)
        analyzeProgress_ = juce::jmax(analyzeProgress_, 0.12f + listenFill_ * 0.75f);
}

void AppState::analyzeLive(juce::AudioBuffer<float> buffer, double sampleRate, const juce::String& name)
{
    if (buffer.getNumSamples() <= 0)
        return;
    runAsync([this, buffer = std::move(buffer), sampleRate, name] {
        auto result = engine::analyze(buffer, sampleRate);
        applyOnMessage([this, result = std::move(result), name]() mutable {
            if (!result.ok())
                return;
            loadedFilename_ = name.toStdString();
            hasTrack_ = true;
            listening_ = hostPlaying_;
            if (analysis_)
                previousAnalysis_ = *analysis_;
            analysis_ = std::move(result.analysis);
            issues_ = std::move(result.issues);
            insights_.clear();
            for (const auto& issue : issues_)
                insights_.push_back(insightFromIssue(issue));
            assistAdvice_ = engine::makeAssist(*analysis_, issues_);
            analysisState_ = AnalysisState::Complete;
            analyzeProgress_ = 1.0f;
            fault_ = {};
            const auto now = juce::Time::currentTimeMillis();
            if (lastLiveSoniMs_ == 0 || now - lastLiveSoniMs_ > 40000)
            {
                lastLiveSoniMs_ = now;
                pushSoni(soni::afterLive(soniContext()));
            }
            persistSession();
            notify();
        });
    });
}

juce::String AppState::nowSection() const
{
    const auto* model = dna();
    if (model == nullptr || !analysis_)
        return hostPlaying_ ? "BODY" : "---";
    const float t = analysis_->durationSec > 0.0f
        ? (float) std::fmod(hostSeconds_, (double) analysis_->durationSec)
        : (float) hostSeconds_;
    for (const auto& section : model->sections)
        if (t >= section.start && t <= section.end)
            return copy::sectionLabel(section.name);
    return hostPlaying_ ? "LIVE" : "---";
}

juce::String AppState::barLabel() const
{
    if (hostBar_ <= 0)
        return hostPlaying_ ? "BAR --" : "WAITING";
    return "BAR " + juce::String(hostBar_);
}

void AppState::dismissSoniMeet()
{
    if (!soniMeetOpen_)
        return;
    soniMeetOpen_ = false;
    metPersisted_ = true;
    persistSession();
    notify();
}

soni::Context AppState::soniContext() const
{
    soni::Context ctx;
    const auto now = juce::Time::getCurrentTime();
    ctx.hour = now.getHours();
    ctx.minute = now.getMinutes();
#ifdef SONORA_IS_PLUGIN
    ctx.plugin = true;
#endif
    ctx.hasTrack = hasTrack_ && (bool) analysis_;
    ctx.analyzing = analysisState_ == AnalysisState::Analyzing || analysisState_ == AnalysisState::Loading;
    ctx.filename = juce::String(loadedFilename_);
    ctx.bpm = bpmLabel();
    ctx.key = keyLabel();
    ctx.style = styleLabel();
    ctx.energy = energyLabel();
    ctx.health = healthScore();
    ctx.live = dawHost_;
    ctx.playing = hostPlaying_;
    ctx.bar = hostBar_;
    ctx.section = nowSection();
    ctx.liveEnergy = liveEnergy_;
    for (const auto& issue : issues_)
    {
        ctx.muddy = ctx.muddy || issue.type == "muddy_low_end";
        ctx.vocalFight = ctx.vocalFight || issue.type == "frequency_conflict";
        ctx.clip = ctx.clip || issue.type == "clipping";
    }
    if (const auto* model = dna())
    {
        for (const auto& section : model->sections)
            if (section.name == "drop")
                ctx.hasDrop = true;
    }
    return ctx;
}

void AppState::pushSoni(const juce::String& text)
{
    if (text.trim().isEmpty())
        return;
    soniMessages_.push_back({ true, text });
    if (soniMessages_.size() > 40)
        soniMessages_.erase(soniMessages_.begin(), soniMessages_.begin() + (int) soniMessages_.size() - 40);
}

void AppState::openSoni()
{
    soniOpen_ = true;
    notify();
}

void AppState::closeSoni()
{
    soniOpen_ = false;
    notify();
}

void AppState::toggleSoni()
{
    soniOpen_ = !soniOpen_;
    if (soniOpen_)
        ensureSoniWelcome();
    notify();
}

void AppState::setSoniMuted(bool muted)
{
    soniMuted_ = muted;
    notify();
}

void AppState::ensureSoniWelcome()
{
    soniOpen_ = true;
    if (soniWelcomed_)
        return;
    soniWelcomed_ = true;
    pushSoni(soni::greet(soniContext()));
    persistSession();
    notify();
}

void AppState::sendSoniChat(const juce::String& text)
{
    const auto line = text.trim();
    if (line.isEmpty())
        return;
    soniOpen_ = true;
    soniMessages_.push_back({ false, line });
    pushSoni(soni::reply(soniContext(), line));
    persistSession();
    notify();
}

bool AppState::trackUnderstood() const
{
    return analysisState_ == AnalysisState::Complete && analysis_.has_value();
}

ListenPhase AppState::listenPhase() const
{
    if (trackUnderstood())
        return ListenPhase::Understood;
    if (!hostPlaying_ && analysisState_ != AnalysisState::Analyzing && analysisState_ != AnalysisState::Loading)
        return ListenPhase::Waiting;
    if (listenFill_ < 0.42f && heardSec_ < 6.0)
        return ListenPhase::Listening;
    return ListenPhase::Mapping;
}

float AppState::listenProgress() const
{
    if (trackUnderstood())
        return 1.0f;
    return juce::jlimit(0.0f, 0.96f, juce::jmax(listenFill_, (float) (heardSec_ / 15.0)));
}

juce::String AppState::listenHeadline() const
{
    switch (listenPhase())
    {
        case ListenPhase::Understood:
            return "TRACK UNDERSTOOD";
        case ListenPhase::Mapping:
            return "SONORA LISTENING";
        case ListenPhase::Listening:
            return hostPlaying_ ? "SONORA LISTENING" : "LISTENING...";
        case ListenPhase::Waiting:
        default:
            return "LISTENING...";
    }
}

juce::String AppState::listenHint() const
{
    switch (listenPhase())
    {
        case ListenPhase::Understood:
            return nowSection() + "    " + barLabel() + "    " + copy::formatTime((float) hostSeconds_);
        case ListenPhase::Mapping:
            return "Structure mapping...";
        case ListenPhase::Listening:
            return liveEnergy_ > 0.02f ? "Energy detected" : "Waiting for the bus";
        case ListenPhase::Waiting:
        default:
            return dawHost_ ? "Waiting for playback    Press Play in FL Studio"
                            : "Load a track. SONORA will listen.";
    }
}

juce::String AppState::structureLine() const
{
    return copy::structureLine(dna());
}

juce::String AppState::mixLine() const
{
    if (!analysis_)
        return hostPlaying_ ? "Mapping the session" : "On the bus";
    return copy::mixLine(*analysis_, issues_);
}

std::vector<juce::String> AppState::sonoraFound() const
{
    if (!analysis_)
        return {};
    return copy::foundLines(*analysis_, issues_);
}

std::vector<LiveMixFlag> AppState::liveMixFlags() const
{
    std::vector<LiveMixFlag> flags;
    const float bass = analysis_ ? (analysis_->bands.sub + analysis_->bands.low)
                                 : juce::jlimit(0.0f, 1.0f, liveEnergy_ * 5.0f);
    bool muddy = false;
    bool clipIssue = false;
    bool narrow = false;
    for (const auto& issue : issues_)
    {
        muddy = muddy || issue.type == "muddy_low_end";
        clipIssue = clipIssue || issue.type == "clipping";
        narrow = narrow || issue.type == "narrow_stereo";
    }
    flags.push_back({ "Bass energy", (muddy || bass >= 0.48f) ? 1 : (bass < 0.22f ? -1 : 0) });
    const float stereo = analysis_ ? analysis_->stereoWidth
                                   : juce::jlimit(0.0f, 1.0f, liveStereo_ * 8.0f);
    flags.push_back({ "Stereo width", (narrow || stereo < 0.18f) ? -1 : (stereo > 0.55f ? 1 : 0) });
    flags.push_back({ "Clipping risk", (clipIssue || livePeak_ > 0.92f) ? 1 : -1 });
    return flags;
}

juce::String AppState::toSessionJson() const
{
    session::Blob blob;
    blob.name = juce::String(loadedFilename_);
    switch (tab_)
    {
        case WorkspaceTab::Understand: blob.tab = "understand"; break;
        case WorkspaceTab::Create: blob.tab = "create"; break;
        case WorkspaceTab::Soni: blob.tab = "soni"; break;
        case WorkspaceTab::Listen:
        default: blob.tab = "listen"; break;
    }
    blob.met = metPersisted_ || !soniMeetOpen_;
    blob.understood = trackUnderstood();
    if (analysis_)
    {
        blob.hasAnalysis = true;
        blob.analysis = *analysis_;
    }
    blob.issues = issues_;
    blob.harmony = harmony_;
    blob.bass = bassClip_;
    blob.pad = padClip_;
    blob.drop = dropPlan_;
    blob.eq = eqProfile_;
    blob.soni = soniMessages_;
    return session::encode(blob);
}

bool AppState::applySessionJson(const juce::String& json)
{
    session::Blob blob;
    if (json.trim().isEmpty() || !session::decode(json, blob))
        return false;
    restoring_ = true;
    if (blob.name.isNotEmpty())
        loadedFilename_ = blob.name.toStdString();
    if (blob.tab == "understand")
        tab_ = WorkspaceTab::Understand;
    else if (blob.tab == "create")
        tab_ = WorkspaceTab::Create;
    else if (blob.tab == "soni")
        tab_ = WorkspaceTab::Soni;
    else
        tab_ = WorkspaceTab::Listen;
    metPersisted_ = blob.met;
    soniMeetOpen_ = !blob.met;
    if (blob.hasAnalysis)
    {
        analysis_ = blob.analysis;
        issues_ = blob.issues;
        insights_.clear();
        for (const auto& issue : issues_)
            insights_.push_back(insightFromIssue(issue));
        if (analysis_)
            assistAdvice_ = engine::makeAssist(*analysis_, issues_);
        hasTrack_ = true;
        analysisState_ = AnalysisState::Complete;
        analyzeProgress_ = 1.0f;
        listenFill_ = 1.0f;
        selectedNode_ = CanvasNode::Understand;
    }
    harmony_ = blob.harmony;
    bassClip_ = blob.bass;
    padClip_ = blob.pad;
    dropPlan_ = blob.drop;
    eqProfile_ = blob.eq;
    if (!blob.soni.empty())
    {
        soniMessages_ = blob.soni;
        soniWelcomed_ = true;
    }
    restoring_ = false;
    notify();
    return true;
}

void AppState::persistSession() const
{
    if (restoring_ || !alive_)
        return;
    session::save(toSessionJson());
}

void AppState::restoreSession()
{
    applySessionJson(session::load());
}

} // namespace sonora
