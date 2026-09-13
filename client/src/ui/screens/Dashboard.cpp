#include "ui/screens/Dashboard.h"

#include "ui/theme/Theme.h"

namespace sonora
{

Dashboard::Dashboard(AppState& state)
    : state_(state)
    , topBar_(state)
    , context_(state)
    , waveform_(state)
    , identity_(state)
    , sonic_(state)
    , arrangement_(state)
    , mixHealth_(state)
    , problems_(state)
    , actions_(state)
    , translation_(state)
    , masking_(state)
    , health_(state)
    , createRail_(state)
    , createPage_(state)
    , referencePage_(state)
    , structure_(state)
    , dnaView_(state)
    , canvas_(state)
    , status_(state)
    , soni_(state)
    , lab_(state)
{
    setOpaque(true);
    setWantsKeyboardFocus(true);

    loadTrack_.setLabel("LOAD NEW TRACK");
    loadTrack_.onClick = [this] { chooseTrack(); };
    listen_.setLabel("LISTEN");
    listen_.onClick = [this] {
        if (captureSite_)
            state_.armCapture(captureSite_());
        if (state_.isListening())
            state_.stopListen();
        else
            state_.startListen();
    };
    reference_.setLabel("REFERENCE");
    reference_.onClick = [this] {
        if (state_.referenceOpen())
            state_.closeReference();
        else
            state_.openReference();
    };
    addAndMakeVisible(listen_);
    addAndMakeVisible(reference_);

    addAndMakeVisible(topBar_);
    addAndMakeVisible(context_);
    addAndMakeVisible(loadTrack_);
    addAndMakeVisible(waveform_);
    addAndMakeVisible(identity_);
    addAndMakeVisible(sonic_);
    addAndMakeVisible(arrangement_);
    addAndMakeVisible(mixHealth_);
    addAndMakeVisible(problems_);
    addAndMakeVisible(actions_);
    addAndMakeVisible(spectrum_);
    addAndMakeVisible(translation_);
    addAndMakeVisible(masking_);
    addAndMakeVisible(health_);
    addAndMakeVisible(createRail_);
    addAndMakeVisible(createPage_);
    addAndMakeVisible(referencePage_);
    addAndMakeVisible(structure_);
    addAndMakeVisible(dnaView_);
    addAndMakeVisible(canvas_);
    addAndMakeVisible(status_);
    addAndMakeVisible(soni_);
    addAndMakeVisible(lab_);
    lab_.setAlwaysOnTop(true);

    state_.addChangeListener(this);
    refreshFromState();
    juce::MessageManager::callAsync([this] { state_.ensureSoniWelcome(); });
}

Dashboard::~Dashboard()
{
    state_.removeChangeListener(this);
}

void Dashboard::setCaptureSite(std::function<void*()> site)
{
    captureSite_ = std::move(site);
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
        if (file.existsAsFile())
            state_.analyzeFile(file);
    });
}

void Dashboard::hideWorkspace()
{
    waveform_.setBounds({});
    identity_.setBounds({});
    sonic_.setBounds({});
    arrangement_.setBounds({});
    mixHealth_.setBounds({});
    problems_.setBounds({});
    actions_.setBounds({});
    spectrum_.setBounds({});
    translation_.setBounds({});
    masking_.setBounds({});
    createPage_.setBounds({});
    referencePage_.setBounds({});
    structure_.setBounds({});
    dnaView_.setBounds({});
}

void Dashboard::refreshFromState()
{
    const auto stage = state_.analysisState();
    const bool complete = stage == AnalysisState::Complete;
    const auto tab = state_.tab();
    const bool listen = complete && tab == WorkspaceTab::Listen && !state_.referenceOpen();
    const bool improve = complete && tab == WorkspaceTab::Improve && !state_.referenceOpen();
    const bool create = complete && tab == WorkspaceTab::Create && !state_.referenceOpen();
    const bool reference = complete && state_.referenceOpen();
    const bool advanced = state_.uiMode() == UiMode::Advanced;

    loadTrack_.setEnabled(stage != AnalysisState::Loading && stage != AnalysisState::Analyzing && !state_.isListening());
    listen_.setVisible(state_.canCapture());
    listen_.setEnabled(stage != AnalysisState::Loading);
    listen_.setLabel(state_.isListening() ? "STOP" : "LISTEN");
    reference_.setVisible(complete);
    reference_.setEnabled(complete);

    waveform_.setVisible(listen);
    identity_.setVisible(listen);
    sonic_.setVisible(listen);
    arrangement_.setVisible(listen);
    mixHealth_.setVisible(improve);
    problems_.setVisible(improve);
    actions_.setVisible(improve);
    spectrum_.setVisible(listen && advanced);
    translation_.setVisible(improve);
    masking_.setVisible(improve);
    health_.setVisible(true);
    createPage_.setVisible(create);
    referencePage_.setVisible(reference);
    structure_.setVisible(create);
    dnaView_.setVisible(improve && advanced);
    canvas_.setVisible(listen || !complete);
    createRail_.setVisible(!create && !state_.soniOpen());
    soni_.setVisible(state_.soniOpen());
    lab_.setVisible(state_.labOpen());

    if (complete && state_.analysis())
        spectrum_.setBands(state_.analysis()->bands);
    else
        spectrum_.clear();

    resized();
    repaint();
    if (!soni_.isEditing())
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
        juce::jmax(200, health_.getX() - context_.getRight() - 32),
        juce::jmax(160, canvas_.getY() - topBar_.getBottom() - 72));
    theme::fillCard(g, stageBounds);
    auto inner = stageBounds.reduced(28, 26);

    if (stage == AnalysisState::Empty)
    {
        g.setColour(colors::muted());
        g.setFont(type::label(10.0f));
        g.drawText("NULLXES SONORA V1.0.1", inner.removeFromTop(14), juce::Justification::centredLeft, true);
        inner.removeFromTop(8);
        g.setColour(colors::foreground());
        g.setFont(type::display(26.0f));
        g.drawText("SONORA", inner.removeFromTop(32), juce::Justification::centredLeft, true);
        inner.removeFromTop(4);
        Theme::drawMuted(g, inner.removeFromTop(16), "ADAPTIVE SOUND INTELLIGENCE");
        inner.removeFromTop(10);
        Theme::drawMuted(g, inner.removeFromTop(14), "FREE VST3");
        inner.removeFromTop(8);
        Theme::drawBody(g, inner.removeFromTop(18), "Track analysis / structure / spectrum / mix diagnostics");
        inner.removeFromTop(8);
        Theme::drawBody(g, inner.removeFromTop(18), "Premium: SONI AI Assistant");
        inner.removeFromTop(16);
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(13.0f));
        g.drawText(state_.canCapture() ? "Load a file, or listen from the DAW."
                                      : "Load a track. SONORA will listen.",
                   inner.removeFromTop(20), juce::Justification::centredLeft, true);
        return;
    }

    if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
    {
        theme::drawSectionLabel(g, inner.removeFromTop(16), state_.isListening() ? "RECORDING" : "LISTENING");
        inner.removeFromTop(12);
        Theme::drawBody(g, inner.removeFromTop(18),
                        state_.isListening() ? "Play the track. Press STOP when you have the section."
                                             : "SONORA is building a model of the track.");
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
    const int canvasH = 110;
    const int leftW = 210;
    const int rightW = juce::jlimit(state_.soniOpen() ? 320 : 280, 380,
                                    juce::roundToInt((float) getWidth() * (state_.soniOpen() ? 0.24f : 0.22f)));

    status_.setBounds(0, getHeight() - statusH, getWidth(), statusH);

    auto body = getLocalBounds().withTrimmedBottom(statusH).reduced(16, 12);
    topBar_.setBounds(body.removeFromTop(64));
    body.removeFromTop(10);

    context_.setBounds(body.removeFromLeft(leftW));
    body.removeFromLeft(14);

    auto right = body.removeFromRight(rightW);
    body.removeFromRight(14);

    health_.setBounds(right.removeFromTop(state_.soniOpen() ? 108 : 132));
    right.removeFromTop(10);
    if (state_.soniOpen())
    {
        soni_.setBounds(right);
        createRail_.setBounds({});
    }
    else
    {
        soni_.setBounds({});
        if (createRail_.isVisible())
        {
            const int createH = juce::jlimit(180, 230, right.getHeight() / 2);
            createRail_.setBounds(right.removeFromBottom(createH));
        }
        else
            createRail_.setBounds({});
    }

    const auto tab = state_.tab();
    const bool complete = state_.analysisState() == AnalysisState::Complete;

    auto loadRow = body.removeFromTop(32);
    loadTrack_.setBounds(loadRow.removeFromRight(148));
    if (complete)
    {
        loadRow.removeFromRight(8);
        reference_.setBounds(loadRow.removeFromRight(110));
    }
    else
        reference_.setBounds({});
    if (state_.canCapture())
    {
        loadRow.removeFromRight(8);
        listen_.setBounds(loadRow.removeFromRight(88));
    }
    else
        listen_.setBounds({});

    lab_.setBounds(getLocalBounds().withTrimmedBottom(statusH));

    if (!complete)
    {
        hideWorkspace();
        canvas_.setBounds(body.removeFromBottom(canvasH));
        return;
    }

    if (state_.referenceOpen())
    {
        hideWorkspace();
        canvas_.setBounds({});
        referencePage_.setBounds(body);
        return;
    }

    if (tab == WorkspaceTab::Create)
    {
        hideWorkspace();
        canvas_.setBounds({});
        auto createArea = body;
        if (state_.dna() != nullptr)
        {
            structure_.setBounds(createArea.removeFromBottom(juce::jlimit(140, 220, createArea.getHeight() / 3)));
            createArea.removeFromBottom(10);
        }
        else
            structure_.setBounds({});
        createPage_.setBounds(createArea);
        return;
    }

    if (tab == WorkspaceTab::Improve)
    {
        hideWorkspace();
        canvas_.setBounds({});
        createPage_.setBounds({});
        referencePage_.setBounds({});
        structure_.setBounds({});

        auto top = body.removeFromTop(juce::jmax(180, body.getHeight() * 46 / 100));
        mixHealth_.setBounds(top.removeFromLeft((top.getWidth() * 54) / 100));
        top.removeFromLeft(10);
        problems_.setBounds(top);
        body.removeFromTop(10);

        if (state_.uiMode() == UiMode::Advanced)
        {
            actions_.setBounds(body.removeFromLeft((body.getWidth() * 34) / 100));
            body.removeFromLeft(10);
            dnaView_.setBounds(body);
            translation_.setBounds({});
            masking_.setBounds({});
        }
        else
        {
            dnaView_.setBounds({});
            actions_.setBounds(body.removeFromLeft((body.getWidth() * 36) / 100));
            body.removeFromLeft(10);
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
    mixHealth_.setBounds({});
    problems_.setBounds({});
    actions_.setBounds({});
    translation_.setBounds({});
    masking_.setBounds({});

    canvas_.setBounds(body.removeFromBottom(canvasH));
    body.removeFromBottom(10);

    waveform_.setBounds(body.removeFromTop(168));
    body.removeFromTop(10);
    identity_.setBounds(body.removeFromTop(86));
    body.removeFromTop(10);

    if (state_.uiMode() == UiMode::Advanced)
    {
        sonic_.setBounds(body.removeFromTop(juce::jmax(92, body.getHeight() / 2)));
        body.removeFromTop(10);
        arrangement_.setBounds(body.removeFromLeft((body.getWidth() * 58) / 100));
        body.removeFromLeft(10);
        spectrum_.setBounds(body);
    }
    else
    {
        spectrum_.setBounds({});
        auto mid = body;
        sonic_.setBounds(mid.removeFromLeft((mid.getWidth() * 52) / 100));
        mid.removeFromLeft(10);
        arrangement_.setBounds(mid);
    }
}

} // namespace sonora
