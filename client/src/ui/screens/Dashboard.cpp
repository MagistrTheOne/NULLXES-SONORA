#include "ui/screens/Dashboard.h"
#include "ui/theme/Theme.h"

namespace sonora
{

Dashboard::Dashboard(AppState& state)
    : state_(state)
{
    setOpaque(true);

    loadTrack_.setLabel("LOAD TRACK");
    loadTrack_.onClick = [this] {
        state_.toggleMockTrack();
        refreshFromState();
    };

    bpm_.setLabel("BPM");
    key_.setLabel("KEY");
    loudness_.setLabel("LOUDNESS");
    insights_.onGenerateReport = [] {};

    addAndMakeVisible(loadTrack_);
    addAndMakeVisible(bpm_);
    addAndMakeVisible(key_);
    addAndMakeVisible(loudness_);
    addAndMakeVisible(spectrum_);
    for (auto& row : issues_)
        addAndMakeVisible(row);
    addAndMakeVisible(insights_);
    addAndMakeVisible(assistant_);
    addAndMakeVisible(status_);

    refreshFromState();
}

void Dashboard::refreshFromState()
{
    if (!state_.hasTrack() || !state_.analysis().has_value())
    {
        bpm_.setEmpty(true);
        key_.setEmpty(true);
        key_.setHint({});
        loudness_.setEmpty(true);
        loudness_.setHint("LUFS (approx)");
        spectrum_.clear();
        issues_[0].setEmpty("No issues  ·  load a track");
        issues_[1].setEmpty({});
        issues_[2].setEmpty({});
        insights_.setInsights({});
        status_.setState(state_.computeState());
        repaint();
        return;
    }

    const auto& analysis = *state_.analysis();
    bpm_.setValue(juce::String(juce::roundToInt(analysis.bpm)));
    bpm_.setHint("tempo");

    if (analysis.key.key.has_value())
    {
        key_.setValue(juce::String(*analysis.key.key));
        key_.setHint(juce::String(juce::roundToInt(analysis.key.confidence * 100.0f)) + "% confidence");
    }
    else
    {
        key_.setEmpty(true);
        key_.setHint("unresolved");
    }

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

    insights_.setInsights(state_.insights());
    status_.setState(state_.computeState());
    repaint();
}

void Dashboard::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());

    const int rail = 72;
    auto left = juce::Rectangle<int>(0, 0, rail, getHeight() - 36);
    g.setColour(colors::border());
    g.fillRect(left.getRight() - 1, left.getY(), 1, left.getHeight());

    auto brand = left.reduced(8, 18);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("NULLXES", brand.removeFromTop(16), juce::Justification::centred, 1);
    brand.removeFromTop(20);

    const char* nav[] = { "ANALYZE", "ASSIST", "PRESETS", "PROJECT", "SETTINGS" };
    for (int i = 0; i < 5; ++i)
    {
        auto item = brand.removeFromTop(28);
        g.setColour(i == 0 ? colors::foreground() : colors::mutedForeground());
        g.setFont(type::label(9.0f));
        g.drawFittedText(nav[i], item, juce::Justification::centred, 1);
    }

    auto header = juce::Rectangle<int>(rail + 20, 16, getWidth() - rail - 40, 70);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("NULLXES", header.removeFromTop(14), juce::Justification::centredLeft, 1);
    g.setColour(colors::foreground());
    g.setFont(type::display(26.0f));
    g.drawFittedText("SONORA", header.removeFromTop(30), juce::Justification::centredLeft, 1);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawFittedText("ADAPTIVE SOUND INTELLIGENCE", header, juce::Justification::centredLeft, 1);

    auto trackLabel = juce::Rectangle<int>(rail + 20, 94, 56, 28);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("TRACK", trackLabel, juce::Justification::centredLeft, 1);

    auto trackName = juce::Rectangle<int>(rail + 76, 94, 360, 28);
    g.setColour(colors::foreground());
    g.setFont(type::body(14.0f));
    g.drawFittedText(
        state_.hasTrack() ? juce::String(state_.loadedFilename()) : "No track loaded",
        trackName,
        juce::Justification::centredLeft,
        1);

    if (bpm_.getWidth() > 0)
    {
        auto intel = bpm_.getBounds().translated(0, -22);
        intel.setHeight(16);
        theme::drawSectionLabel(g, intel, "TRACK INTELLIGENCE");
    }

    if (issues_[0].getWidth() > 0)
    {
        auto issuesHeader = issues_[0].getBounds().translated(0, -20);
        issuesHeader.setHeight(16);
        theme::drawSectionLabel(g, issuesHeader, "ISSUES DETECTED");
    }
}

void Dashboard::resized()
{
    const int rail = 72;
    const int statusH = 36;
    const int rightW = juce::jlimit(280, 360, juce::roundToInt((float) getWidth() * 0.26f));

    status_.setBounds(0, getHeight() - statusH, getWidth(), statusH);

    auto content = getLocalBounds().withTrimmedLeft(rail).withTrimmedBottom(statusH).reduced(20, 16);
    auto right = content.removeFromRight(rightW);
    content.removeFromRight(16);

    content.removeFromTop(70);
    content.removeFromTop(8);
    auto track = content.removeFromTop(28);
    loadTrack_.setBounds(track.removeFromRight(128));

    content.removeFromTop(14);
    content.removeFromTop(16);
    auto metrics = content.removeFromTop(86);
    const int gap = 10;
    const int cardW = juce::jmax(80, (metrics.getWidth() - gap * 2) / 3);
    bpm_.setBounds(metrics.removeFromLeft(cardW));
    metrics.removeFromLeft(gap);
    key_.setBounds(metrics.removeFromLeft(cardW));
    metrics.removeFromLeft(gap);
    loudness_.setBounds(metrics);

    content.removeFromTop(14);
    const int spectrumH = juce::jlimit(130, 220, content.getHeight() / 2 - 10);
    spectrum_.setBounds(content.removeFromTop(spectrumH));
    content.removeFromTop(14);
    content.removeFromTop(18);

    const int rowH = juce::jmax(34, content.getHeight() / 3);
    for (auto& row : issues_)
        row.setBounds(content.removeFromTop(rowH));

    const int assistantH = juce::jlimit(210, 270, right.getHeight() / 2);
    assistant_.setBounds(right.removeFromBottom(assistantH));
    right.removeFromBottom(12);
    insights_.setBounds(right);
}

} // namespace sonora
