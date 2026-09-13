#include "ui/components/SoniPanel.h"

#include "soni/SoniVoice.h"
#include "ui/theme/Theme.h"

#include <cmath>

namespace sonora
{

SoniPanel::SoniPanel(AppState& state)
    : state_(state)
{
    input_.setMultiLine(false);
    input_.setReturnKeyStartsNewLine(false);
    input_.setEscapeAndReturnKeysConsumed(true);
    input_.setTextToShowWhenEmpty("пиши как есть. не как боту.", colors::mutedForeground());
    input_.setColour(juce::TextEditor::backgroundColourId, colors::elevated());
    input_.setColour(juce::TextEditor::textColourId, colors::foreground());
    input_.setColour(juce::TextEditor::outlineColourId, colors::border());
    input_.setColour(juce::TextEditor::focusedOutlineColourId, colors::borderStrong());
    input_.setColour(juce::TextEditor::highlightColourId, colors::borderStrong());
    input_.setFont(type::body(13.0f));
    input_.addListener(this);
    addAndMakeVisible(input_);

    send_.setLabel("SEND");
    send_.onClick = [this] { send(); };
    addAndMakeVisible(send_);

    mute_.setLabel(state_.soniMuted() ? "MUTED" : "VOICE");
    mute_.onClick = [this] {
        state_.setSoniMuted(!state_.soniMuted());
        if (state_.soniMuted())
            soni::Voice::get().silence();
    };
    addAndMakeVisible(mute_);

    spokenCount_ = (int) state_.soniMessages().size();
    state_.addChangeListener(this);
    startTimerHz(20);
}

SoniPanel::~SoniPanel()
{
    stopTimer();
    state_.removeChangeListener(this);
}

bool SoniPanel::isEditing() const
{
    return input_.hasKeyboardFocus(true);
}

void SoniPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    mute_.setLabel(state_.soniMuted() ? "MUTED" : "VOICE");
    speakLatest();
    resized();
    repaint();
}

void SoniPanel::textEditorReturnKeyPressed(juce::TextEditor&)
{
    send();
}

void SoniPanel::timerCallback()
{
    pulse_ += 0.12f;
    if (soni::Voice::get().speaking())
        repaint(getLocalBounds().removeFromTop(52));
}

void SoniPanel::send()
{
    const auto text = input_.getText();
    if (text.trim().isEmpty())
        return;
    input_.clear();
    state_.sendSoniChat(text);
}

void SoniPanel::speakLatest()
{
    const auto& lines = state_.soniMessages();
    if ((int) lines.size() <= spokenCount_)
        return;
    spokenCount_ = (int) lines.size();
    if (state_.soniMuted() || lines.empty() || !lines.back().fromSoni)
        return;
    soni::Voice::get().speak(lines.back().text);
}

void SoniPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 14);
    auto header = bounds.removeFromTop(46);
    Theme::drawMuted(g, header.removeFromTop(12), "PREMIUM");
    g.setColour(colors::foreground());
    g.setFont(type::display(20.0f));
    auto title = header.removeFromTop(22);
    g.drawText("SONI", title.removeFromLeft(70), juce::Justification::centredLeft, true);
    const bool live = soni::Voice::get().speaking();
    const float glow = live ? (0.45f + 0.55f * (0.5f + 0.5f * std::sin(pulse_))) : 0.2f;
    auto dot = title.removeFromLeft(14).withSizeKeepingCentre(8, 8);
    g.setColour(colors::foreground().withAlpha(glow));
    g.fillEllipse(dot.toFloat());
    Theme::drawMuted(g, header, "AI Assistant");

    bounds.removeFromBottom(44);
    bounds.removeFromBottom(8);

    const auto& lines = state_.soniMessages();
    int y = bounds.getBottom();
    for (int i = (int) lines.size() - 1; i >= 0 && y > bounds.getY(); --i)
    {
        const auto& line = lines[(size_t) i];
        const int textH = juce::jmax(32, (int) std::ceil((double) line.text.length() / 34.0) * 16 + 16);
        y -= textH + 8;
        if (y + textH < bounds.getY())
            break;
        auto bubble = juce::Rectangle<int>(bounds.getX(), juce::jmax(bounds.getY(), y), bounds.getWidth(), textH);
        if (!line.fromSoni)
            bubble = bubble.removeFromRight(juce::jmax(80, bubble.getWidth() * 4 / 5));
        g.setColour(line.fromSoni ? colors::elevated() : colors::card());
        g.fillRect(bubble);
        g.setColour(colors::border());
        g.drawRect(bubble, 1);
        auto inner = bubble.reduced(8, 6);
        g.setColour(colors::mutedForeground());
        g.setFont(type::label(9.0f));
        g.drawText(line.fromSoni ? "SONI" : "YOU", inner.removeFromTop(12), juce::Justification::centredLeft, true);
        g.setColour(colors::foreground());
        g.setFont(type::body(13.0f));
        g.drawMultiLineText(line.text, inner.getX(), inner.getY() + 12, inner.getWidth());
    }
}

void SoniPanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    auto header = bounds.removeFromTop(46);
    mute_.setBounds(header.removeFromRight(70).removeFromBottom(26));
    bounds.removeFromTop(6);
    auto row = bounds.removeFromBottom(36);
    send_.setBounds(row.removeFromRight(72));
    row.removeFromRight(8);
    input_.setBounds(row);
}

} // namespace sonora
