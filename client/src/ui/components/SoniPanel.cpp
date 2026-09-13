#include "ui/components/SoniPanel.h"

#include "ui/theme/Theme.h"

namespace sonora
{

SoniPanel::SoniPanel(AppState& state)
    : state_(state)
{
    face_.setMode(SoniFace::Mode::Chat);
    addAndMakeVisible(face_);
    input_.setMultiLine(false);
    input_.setReturnKeyStartsNewLine(false);
    input_.setEscapeAndReturnKeysConsumed(true);
    input_.setTextToShowWhenEmpty("...", colors::mutedForeground());
    input_.setColour(juce::TextEditor::backgroundColourId, colors::background().withAlpha(0.72f));
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
    state_.addChangeListener(this);
}

SoniPanel::~SoniPanel()
{
    state_.removeChangeListener(this);
}

bool SoniPanel::isEditing() const
{
    return input_.hasKeyboardFocus(true);
}

void SoniPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    resized();
    repaint();
}

void SoniPanel::textEditorReturnKeyPressed(juce::TextEditor&)
{
    send();
}

void SoniPanel::send()
{
    const auto text = input_.getText();
    if (text.trim().isEmpty())
        return;
    input_.clear();
    state_.sendSoniChat(text);
}

void SoniPanel::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());
}

void SoniPanel::paintOverChildren(juce::Graphics& g)
{
    auto chrome = getLocalBounds();
    chrome.removeFromBottom(70);
    auto fade = chrome.removeFromBottom(90).toFloat();
    juce::ColourGradient wash(colors::background().withAlpha(0.0f), fade.getX(), fade.getY(),
                              colors::background().withAlpha(0.82f), fade.getX(), fade.getBottom(), false);
    g.setGradientFill(wash);
    g.fillRect(fade);

    auto header = getLocalBounds().reduced(20, 18).removeFromTop(36);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    g.drawText("SONI", header.removeFromRight(90), juce::Justification::centredRight, true);
}

void SoniPanel::resized()
{
    face_.setBounds(getLocalBounds());
    auto row = getLocalBounds().reduced(20, 18).removeFromBottom(36);
    send_.setBounds(row.removeFromRight(72));
    row.removeFromRight(8);
    input_.setBounds(row);
}

} // namespace sonora
