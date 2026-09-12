#include "ui/screens/Dashboard.h"

#include "backend/ClientLog.h"
#include "ui/theme/Theme.h"

namespace sonora
{

Dashboard::Dashboard(AppState& state)
    : state_(state)
    , topBar_(state)
    , context_(state)
    , waveform_(state)
    , identity_(state)
    , arrangement_(state)
    , mixHealth_(state)
    , translation_(state)
    , masking_(state)
    , health_(state)
    , assist_(state)
    , createRail_(state)
    , createPage_(state)
    , referencePage_(state)
    , structure_(state)
    , dnaView_(state)
    , canvas_(state)
    , status_(state)
{
    setOpaque(true);
    setWantsKeyboardFocus(true);

    loadTrack_.setLabel("LOAD NEW TRACK");
    loadTrack_.onClick = [this] { chooseTrack(); };

    addAndMakeVisible(topBar_);
    addAndMakeVisible(context_);
    addAndMakeVisible(loadTrack_);
    addAndMakeVisible(waveform_);
    addAndMakeVisible(identity_);
    addAndMakeVisible(arrangement_);
    addAndMakeVisible(mixHealth_);
    addAndMakeVisible(spectrum_);
    addAndMakeVisible(translation_);
    addAndMakeVisible(masking_);
    addAndMakeVisible(health_);
    addAndMakeVisible(assist_);
    addAndMakeVisible(createRail_);
    addAndMakeVisible(createPage_);
    addAndMakeVisible(referencePage_);
    addAndMakeVisible(structure_);
    addAndMakeVisible(dnaView_);
    addAndMakeVisible(canvas_);
    addAndMakeVisible(status_);

    state_.addChangeListener(this);
    refreshFromState();
}

Dashboard::~Dashboard()
{
    state_.removeChangeListener(this);
}

void Dashboard::changeListenerCallback(juce::ChangeBroadcaster*)
{
    refreshFromState();
}

bool Dashboard::keyPressed(const juce::KeyPress& key)
{
    return state_.handleKeyPress(key);
}

void Dashboard::chooseTrack()
{
    chooser_ = std::make_unique<juce::FileChooser>(
        "LOAD TRACK",
        juce::File(),
        "*.wav;*.mp3;*.flac");
    constexpr auto chooserFlags = juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles;
    chooser_->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser) {
        const auto file = chooser.getResult();
        clientLog("FileChooser result=" + file.getFullPathName());
        if (file.existsAsFile())
            state_.analyzeFile(file);
    });
}

void Dashboard::refreshFromState()
{
    const auto stage = state_.analysisState();
    const bool complete = stage == AnalysisState::Complete;
    const auto tab = state_.tab();
    const bool track = complete && tab == WorkspaceTab::Track;
    const bool mix = complete && tab == WorkspaceTab::Mix;
    const bool arrangement = complete && tab == WorkspaceTab::Arrangement;
    const bool create = complete && tab == WorkspaceTab::Create;
    const bool reference = complete && tab == WorkspaceTab::Reference;
    const bool advancedMix = mix && state_.uiMode() == UiMode::Advanced;

    loadTrack_.setEnabled(stage != AnalysisState::Loading && stage != AnalysisState::Analyzing);
    waveform_.setVisible(track);
    identity_.setVisible(track);
    arrangement_.setVisible(track);
    mixHealth_.setVisible(track || mix);
    spectrum_.setVisible(track);
    translation_.setVisible(track || mix);
    masking_.setVisible(track || mix);
    health_.setVisible(true);
    createPage_.setVisible(create);
    referencePage_.setVisible(reference);
    structure_.setVisible(arrangement);
    dnaView_.setVisible(advancedMix);
    canvas_.setVisible(track || !complete);

    if (complete && state_.analysis())
        spectrum_.setBands(state_.analysis()->bands);
    else
        spectrum_.clear();

    resized();
    repaint();
    grabKeyboardFocus();
}

void Dashboard::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());

    const auto stage = state_.analysisState();
    if (stage == AnalysisState::Complete)
        return;

    auto stageBounds = juce::Rectangle<int>(
        context_.getRight() + 16,
        topBar_.getBottom() + 56,
        juce::jmax(200, assist_.getX() - context_.getRight() - 32),
        juce::jmax(120, canvas_.getY() - topBar_.getBottom() - 72));
    theme::fillCard(g, stageBounds);
    auto inner = stageBounds.reduced(24, 24);

    if (stage == AnalysisState::Empty)
    {
        theme::drawSectionLabel(g, inner.removeFromTop(16), "NO TRACK");
        inner.removeFromTop(10);
        g.setColour(colors::foreground());
        g.setFont(type::display(22.0f));
        g.drawText("Load a track. SONORA will listen.", inner.removeFromTop(28), juce::Justification::centredLeft, true);
        inner.removeFromTop(10);
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(13.0f));
        g.drawText("Then we can understand it, improve it, and create.", inner.removeFromTop(20),
                   juce::Justification::centredLeft, true);
        return;
    }

    if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
    {
        theme::drawSectionLabel(g, inner.removeFromTop(16), "LISTENING");
        inner.removeFromTop(12);
        Theme::drawBody(g, inner.removeFromTop(18), "SONORA is building a model of the track.");
        inner.removeFromTop(14);
        auto bar = inner.removeFromTop(10);
        g.setColour(colors::border());
        g.fillRect(bar);
        g.setColour(colors::foreground());
        g.fillRect(bar.withWidth(juce::jmax(4, juce::roundToInt((float) bar.getWidth() * state_.analyzeProgress()))));
        return;
    }

    const auto& fault = state_.fault();
    theme::drawSectionLabel(g, inner.removeFromTop(16), fault.title.isEmpty() ? "FAILED" : fault.title);
    inner.removeFromTop(12);
    g.setColour(colors::destructive());
    g.setFont(type::body(13.0f));
    g.drawMultiLineText(
        fault.reason.isEmpty() ? "unknown error" : fault.reason,
        inner.getX(),
        inner.getY() + 14,
        inner.getWidth());
}

void Dashboard::resized()
{
    const int statusH = 36;
    const int canvasH = 118;
    const int leftW = 210;
    const int rightW = juce::jlimit(280, 340, juce::roundToInt((float) getWidth() * 0.22f));

    status_.setBounds(0, getHeight() - statusH, getWidth(), statusH);

    auto body = getLocalBounds().withTrimmedBottom(statusH).reduced(16, 12);
    topBar_.setBounds(body.removeFromTop(64));
    body.removeFromTop(10);

    context_.setBounds(body.removeFromLeft(leftW));
    body.removeFromLeft(14);

    auto right = body.removeFromRight(rightW);
    body.removeFromRight(14);

    health_.setBounds(right.removeFromTop(132));
    right.removeFromTop(10);
    const int createH = juce::jlimit(180, 230, right.getHeight() / 3);
    createRail_.setBounds(right.removeFromBottom(createH));
    right.removeFromBottom(10);
    assist_.setBounds(right);

    const auto tab = state_.tab();
    const bool complete = state_.analysisState() == AnalysisState::Complete;

    auto loadRow = body.removeFromTop(32);
    loadTrack_.setBounds(loadRow.removeFromRight(148));

    if (!complete)
    {
        waveform_.setBounds({});
        identity_.setBounds({});
        arrangement_.setBounds({});
        mixHealth_.setBounds({});
        spectrum_.setBounds({});
        translation_.setBounds({});
        masking_.setBounds({});
        createPage_.setBounds({});
        referencePage_.setBounds({});
        structure_.setBounds({});
        dnaView_.setBounds({});
        canvas_.setBounds(body.removeFromBottom(canvasH));
        return;
    }

    if (tab == WorkspaceTab::Create)
    {
        waveform_.setBounds({});
        identity_.setBounds({});
        arrangement_.setBounds({});
        mixHealth_.setBounds({});
        spectrum_.setBounds({});
        translation_.setBounds({});
        masking_.setBounds({});
        structure_.setBounds({});
        dnaView_.setBounds({});
        canvas_.setBounds({});
        referencePage_.setBounds({});
        createPage_.setBounds(body);
        return;
    }

    if (tab == WorkspaceTab::Reference)
    {
        waveform_.setBounds({});
        identity_.setBounds({});
        arrangement_.setBounds({});
        mixHealth_.setBounds({});
        spectrum_.setBounds({});
        translation_.setBounds({});
        masking_.setBounds({});
        structure_.setBounds({});
        dnaView_.setBounds({});
        canvas_.setBounds({});
        createPage_.setBounds({});
        referencePage_.setBounds(body);
        return;
    }

    if (tab == WorkspaceTab::Arrangement)
    {
        waveform_.setBounds({});
        identity_.setBounds({});
        arrangement_.setBounds({});
        mixHealth_.setBounds({});
        spectrum_.setBounds({});
        translation_.setBounds({});
        masking_.setBounds({});
        createPage_.setBounds({});
        referencePage_.setBounds({});
        dnaView_.setBounds({});
        canvas_.setBounds({});
        structure_.setBounds(body);
        return;
    }

    if (tab == WorkspaceTab::Mix)
    {
        waveform_.setBounds({});
        identity_.setBounds({});
        arrangement_.setBounds({});
        spectrum_.setBounds({});
        createPage_.setBounds({});
        referencePage_.setBounds({});
        structure_.setBounds({});
        canvas_.setBounds({});
        if (state_.uiMode() == UiMode::Advanced)
        {
            auto mixTop = body.removeFromTop(juce::jmax(180, body.getHeight() / 2));
            mixHealth_.setBounds(mixTop.removeFromLeft(mixTop.getWidth() / 2 - 6));
            mixTop.removeFromLeft(12);
            auto side = mixTop;
            translation_.setBounds(side.removeFromTop(side.getHeight() / 2 - 6));
            side.removeFromTop(12);
            masking_.setBounds(side);
            body.removeFromTop(10);
            dnaView_.setBounds(body);
        }
        else
        {
            dnaView_.setBounds({});
            mixHealth_.setBounds(body.removeFromTop(juce::jmax(160, body.getHeight() / 2)));
            body.removeFromTop(10);
            translation_.setBounds(body.removeFromLeft(body.getWidth() / 2 - 6));
            body.removeFromLeft(12);
            masking_.setBounds(body);
        }
        return;
    }

    createPage_.setBounds({});
    referencePage_.setBounds({});
    structure_.setBounds({});
    dnaView_.setBounds({});
    canvas_.setBounds(body.removeFromBottom(canvasH));
    body.removeFromBottom(10);

    waveform_.setBounds(body.removeFromTop(92));
    body.removeFromTop(10);
    identity_.setBounds(body.removeFromTop(86));
    body.removeFromTop(10);

    auto mid = body.removeFromTop(juce::jmax(120, body.getHeight() / 2));
    arrangement_.setBounds(mid.removeFromLeft((mid.getWidth() * 58) / 100));
    mid.removeFromLeft(10);
    mixHealth_.setBounds(mid);
    body.removeFromTop(10);

    const int col = (body.getWidth() - 20) / 3;
    spectrum_.setBounds(body.removeFromLeft(col));
    body.removeFromLeft(10);
    translation_.setBounds(body.removeFromLeft(col));
    body.removeFromLeft(10);
    masking_.setBounds(body);
}

} // namespace sonora
