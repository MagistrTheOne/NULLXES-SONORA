#include "ui/screens/Dashboard.h"

#include "ui/theme/Theme.h"

namespace sonora
{

Dashboard::Dashboard(AppState& state)
    : state_(state)
    , context_(state)
    , tabs_(state)
    , insights_(state)
    , create_(state)
    , canvas_(state)
    , status_(state)
{
    setOpaque(true);

    loadTrack_.setLabel("LOAD TRACK");
    loadTrack_.onClick = [this] { chooseTrack(); };

    bpm_.setLabel("BPM");
    key_.setLabel("KEY");
    loudness_.setLabel("LOUDNESS");

    addAndMakeVisible(context_);
    addAndMakeVisible(tabs_);
    addAndMakeVisible(loadTrack_);
    addAndMakeVisible(bpm_);
    addAndMakeVisible(key_);
    addAndMakeVisible(loudness_);
    addAndMakeVisible(spectrum_);
    for (auto& row : issues_)
        addAndMakeVisible(row);
    addAndMakeVisible(insights_);
    addAndMakeVisible(create_);
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

void Dashboard::chooseTrack()
{
    chooser_ = std::make_unique<juce::FileChooser>(
        "LOAD TRACK",
        juce::File(),
        "*.wav;*.mp3;*.flac;*.aiff;*.aif");
    constexpr auto flags = juce::FileBrowserComponent::openMode
                           | juce::FileBrowserComponent::canSelectFiles;
    chooser_->launchAsync(flags, [this](const juce::FileChooser& chooser) {
        const auto file = chooser.getResult();
        if (file.existsAsFile())
            state_.analyzeFile(file);
    });
}

void Dashboard::refreshFromState()
{
    const auto stage = state_.analysisState();
    const bool complete = stage == AnalysisState::Complete;
    const auto tab = state_.tab();

    loadTrack_.setEnabled(stage != AnalysisState::Loading && stage != AnalysisState::Analyzing);
    loadTrack_.setLabel(stage == AnalysisState::Empty ? "LOAD TRACK" : "LOAD TRACK");

    const bool showMetrics = complete && tab == WorkspaceTab::Overview;
    const bool showSpectrum = complete && (tab == WorkspaceTab::Overview || tab == WorkspaceTab::Spectrum);
    const bool showIssues = complete && tab == WorkspaceTab::Overview;

    bpm_.setVisible(showMetrics);
    key_.setVisible(showMetrics);
    loudness_.setVisible(showMetrics);
    spectrum_.setVisible(showSpectrum);
    for (auto& row : issues_)
        row.setVisible(showIssues);

    if (complete && state_.analysis())
    {
        const auto& analysis = *state_.analysis();
        bpm_.setValue(state_.bpmLabel());
        bpm_.setHint("tempo");
        key_.setValue(state_.keyLabel());
        if (analysis.key.key.has_value())
            key_.setHint(juce::String(juce::roundToInt(analysis.key.confidence * 100.0f)) + "% confidence");
        else
            key_.setHint("unresolved");
        loudness_.setValue(juce::String(analysis.loudnessLufsApprox, 1));
        loudness_.setHint("LUFS (approx)");
        spectrum_.setBands(analysis.bands);

        const auto& issueList = state_.issues();
        for (size_t i = 0; i < issues_.size(); ++i)
        {
            if (i < issueList.size())
                issues_[i].setIssue(issueList[i]);
            else
                issues_[i].setEmpty({});
        }
    }
    else
    {
        bpm_.setEmpty(true);
        key_.setEmpty(true);
        loudness_.setEmpty(true);
        loudness_.setHint("LUFS (approx)");
        spectrum_.clear();
        issues_[0].setEmpty("No issues  ·  load a track");
        issues_[1].setEmpty({});
        issues_[2].setEmpty({});
    }

    resized();
    repaint();
}

void Dashboard::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());

    auto header = juce::Rectangle<int>(context_.getRight() + 20, 16, getWidth() - context_.getWidth() - 360, 70);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("NULLXES", header.removeFromTop(14), juce::Justification::centredLeft, 1);
    g.setColour(colors::foreground());
    g.setFont(type::display(26.0f));
    g.drawFittedText("SONORA", header.removeFromTop(30), juce::Justification::centredLeft, 1);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawFittedText("ADAPTIVE SOUND INTELLIGENCE", header, juce::Justification::centredLeft, 1);

    auto trackRow = juce::Rectangle<int>(context_.getRight() + 20, tabs_.getBottom() + 10, 420, 22);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("TRACK", trackRow.removeFromLeft(56), juce::Justification::centredLeft, 1);
    g.setColour(colors::foreground());
    g.setFont(type::body(14.0f));
    g.drawFittedText(
        state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK",
        trackRow,
        juce::Justification::centredLeft,
        1);

    const auto stage = state_.analysisState();
    const auto tab = state_.tab();
    auto stageBounds = juce::Rectangle<int>(
        context_.getRight() + 20,
        loadTrack_.getBottom() + 18,
        canvas_.getX() > 0 ? insights_.getX() - context_.getRight() - 36 : getWidth() / 2,
        juce::jmax(80, canvas_.getY() - loadTrack_.getBottom() - 28));

    if (stage != AnalysisState::Complete)
    {
        g.setColour(colors::card());
        g.fillRoundedRectangle(stageBounds.toFloat(), 10.0f);
        auto inner = stageBounds.reduced(24, 28);

        if (stage == AnalysisState::Empty)
        {
            theme::drawSectionLabel(g, inner.removeFromTop(16), "NO TRACK");
            inner.removeFromTop(10);
            g.setColour(colors::foreground());
            g.setFont(type::display(22.0f));
            g.drawFittedText("LOAD TRACK", inner.removeFromTop(28), juce::Justification::centredLeft, 1);
            inner.removeFromTop(8);
            g.setColour(colors::mutedForeground());
            g.setFont(type::body(13.0f));
            g.drawFittedText(
                "SONORA starts when a file enters the engine.",
                inner.removeFromTop(20),
                juce::Justification::centredLeft,
                1);
        }
        else if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
        {
            theme::drawSectionLabel(g, inner.removeFromTop(16), "ANALYZING AUDIO");
            inner.removeFromTop(14);
            auto bar = inner.removeFromTop(10).toFloat();
            g.setColour(colors::border());
            g.fillRoundedRectangle(bar, 3.0f);
            g.setColour(colors::foreground());
            g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * state_.analyzeProgress()), 3.0f);
            inner.removeFromTop(16);
            g.setColour(colors::mutedForeground());
            g.setFont(type::label(11.0f));
            g.drawFittedText("DSP ENGINE", inner.removeFromTop(16), juce::Justification::centredLeft, 1);
        }
        else
        {
            theme::drawSectionLabel(g, inner.removeFromTop(16), "FAILED");
            inner.removeFromTop(10);
            g.setColour(colors::destructive());
            g.setFont(type::body(13.0f));
            g.drawFittedText(
                state_.lastError().isEmpty() ? "Backend did not complete analysis." : state_.lastError(),
                inner.removeFromTop(40),
                juce::Justification::topLeft,
                3);
        }
        return;
    }

    if (bpm_.isVisible())
    {
        auto intel = bpm_.getBounds().translated(0, -20);
        intel.setHeight(16);
        theme::drawSectionLabel(g, intel, "TRACK INTELLIGENCE");
    }

    if (issues_[0].isVisible())
    {
        auto issuesHeader = issues_[0].getBounds().translated(0, -20);
        issuesHeader.setHeight(16);
        theme::drawSectionLabel(g, issuesHeader, "ISSUES DETECTED");
    }

    if (tab == WorkspaceTab::Harmony)
    {
        g.setColour(colors::card());
        g.fillRoundedRectangle(stageBounds.toFloat(), 10.0f);
        auto inner = stageBounds.reduced(24, 22);
        theme::drawSectionLabel(g, inner.removeFromTop(16), "HARMONY");
        inner.removeFromTop(10);
        if (state_.harmony())
        {
            g.setColour(colors::foreground());
            g.setFont(type::display(20.0f));
            g.drawFittedText(
                juce::String(state_.harmony()->key) + "  ·  " + juce::String(state_.harmony()->bars) + " bars",
                inner.removeFromTop(28),
                juce::Justification::centredLeft,
                1);
            inner.removeFromTop(8);
            juce::String line;
            for (const auto& chord : state_.harmony()->chords)
            {
                if (line.isNotEmpty())
                    line << "   ";
                line << juce::String(chord);
            }
            g.setFont(type::body(16.0f));
            g.drawFittedText(line, inner.removeFromTop(24), juce::Justification::centredLeft, 1);
        }
        else
        {
            g.setColour(colors::mutedForeground());
            g.setFont(type::body(13.0f));
            g.drawFittedText("CREATE → CHORDS writes a MIDI-first object.", inner.removeFromTop(24), juce::Justification::centredLeft, 1);
        }
    }
    else if (tab == WorkspaceTab::Generate)
    {
        g.setColour(colors::card());
        g.fillRoundedRectangle(stageBounds.toFloat(), 10.0f);
        auto inner = stageBounds.reduced(24, 22);
        theme::drawSectionLabel(g, inner.removeFromTop(16), "GENERATE");
        inner.removeFromTop(10);
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(13.0f));
        g.drawFittedText(
            "SONORA does not chat. The creative engine on the right produces objects.",
            inner.removeFromTop(40),
            juce::Justification::topLeft,
            2);
    }
    else if (tab == WorkspaceTab::Mix || tab == WorkspaceTab::Master)
    {
        g.setColour(colors::card());
        g.fillRoundedRectangle(stageBounds.toFloat(), 10.0f);
        auto inner = stageBounds.reduced(24, 22);
        theme::drawSectionLabel(g, inner.removeFromTop(16), tab == WorkspaceTab::Mix ? "MIX" : "MASTER");
        inner.removeFromTop(10);
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(13.0f));
        g.drawFittedText("Next module. Intelligence and objects come first.", inner.removeFromTop(24), juce::Justification::centredLeft, 1);
    }
}

void Dashboard::resized()
{
    const int statusH = 36;
    const int canvasH = 132;
    const int leftW = 220;
    const int rightW = juce::jlimit(280, 340, juce::roundToInt((float) getWidth() * 0.24f));

    status_.setBounds(0, getHeight() - statusH, getWidth(), statusH);

    auto body = getLocalBounds().withTrimmedBottom(statusH).reduced(16, 14);
    canvas_.setBounds(body.removeFromBottom(canvasH));
    body.removeFromBottom(12);

    context_.setBounds(body.removeFromLeft(leftW));
    body.removeFromLeft(16);

    auto right = body.removeFromRight(rightW);
    body.removeFromRight(16);

    const int createH = juce::jlimit(250, 320, right.getHeight() / 2);
    create_.setBounds(right.removeFromBottom(createH));
    right.removeFromBottom(12);
    insights_.setBounds(right);

    body.removeFromTop(72);
    tabs_.setBounds(body.removeFromTop(30));
    body.removeFromTop(10);
    auto track = body.removeFromTop(28);
    loadTrack_.setBounds(track.removeFromRight(128));

    const auto tab = state_.tab();
    const bool complete = state_.analysisState() == AnalysisState::Complete;
    if (!complete || tab == WorkspaceTab::Harmony || tab == WorkspaceTab::Generate
        || tab == WorkspaceTab::Mix || tab == WorkspaceTab::Master)
    {
        bpm_.setBounds({});
        key_.setBounds({});
        loudness_.setBounds({});
        if (!(complete && tab == WorkspaceTab::Spectrum))
            spectrum_.setBounds({});
        for (auto& row : issues_)
            row.setBounds({});
        if (complete && tab == WorkspaceTab::Spectrum)
        {
            body.removeFromTop(12);
            spectrum_.setBounds(body);
        }
        return;
    }

    body.removeFromTop(14);
    body.removeFromTop(16);
    auto metrics = body.removeFromTop(86);
    const int gap = 10;
    const int cardW = juce::jmax(80, (metrics.getWidth() - gap * 2) / 3);
    bpm_.setBounds(metrics.removeFromLeft(cardW));
    metrics.removeFromLeft(gap);
    key_.setBounds(metrics.removeFromLeft(cardW));
    metrics.removeFromLeft(gap);
    loudness_.setBounds(metrics);

    if (tab == WorkspaceTab::Spectrum)
    {
        body.removeFromTop(12);
        spectrum_.setBounds(body);
        for (auto& row : issues_)
            row.setBounds({});
        return;
    }

    body.removeFromTop(14);
    const int spectrumH = juce::jlimit(110, 180, body.getHeight() / 2 - 8);
    spectrum_.setBounds(body.removeFromTop(spectrumH));
    body.removeFromTop(14);
    body.removeFromTop(18);
    const int rowH = juce::jmax(32, body.getHeight() / 3);
    for (auto& row : issues_)
        row.setBounds(body.removeFromTop(rowH));
}

} // namespace sonora
