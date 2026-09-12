#include "ui/components/AssistantPanel.h"
#include "ui/theme/Theme.h"

namespace sonora
{

AssistantPanel::AssistantPanel()
{
    input_.setMultiLine(true, true);
    input_.setReturnKeyStartsNewLine(true);
    input_.setColour(juce::TextEditor::backgroundColourId, colors::elevated());
    input_.setColour(juce::TextEditor::textColourId, colors::foreground());
    input_.setColour(juce::TextEditor::outlineColourId, colors::border());
    input_.setColour(juce::TextEditor::focusedOutlineColourId, colors::borderStrong());
    input_.setColour(juce::TextEditor::highlightColourId, colors::borderStrong());
    input_.setFont(type::body(13.0f));
    input_.setTextToShowWhenEmpty("Describe sound direction...", colors::mutedForeground());
    addAndMakeVisible(input_);

    chords_.setLabel("GENERATE CHORDS");
    bassline_.setLabel("GENERATE BASSLINE");
    pad_.setLabel("GENERATE PAD");
    arrangement_.setLabel("SUGGEST ARRANGEMENT");
    chords_.setVariant(ActionButton::Variant::Ghost);
    bassline_.setVariant(ActionButton::Variant::Ghost);
    pad_.setVariant(ActionButton::Variant::Ghost);
    arrangement_.setVariant(ActionButton::Variant::Ghost);

    addAndMakeVisible(chords_);
    addAndMakeVisible(bassline_);
    addAndMakeVisible(pad_);
    addAndMakeVisible(arrangement_);
}

void AssistantPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto header = getLocalBounds().reduced(16, 14).removeFromTop(36);
    g.setColour(colors::muted());
    g.setFont(type::label(10.0f));
    g.drawFittedText("SONORA", header.removeFromTop(14), juce::Justification::centredLeft, 1);
    g.setColour(colors::mutedForeground());
    g.setFont(type::body(11.0f));
    g.drawFittedText("Visual only  ·  no model attached", header, juce::Justification::centredLeft, 1);
}

void AssistantPanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    bounds.removeFromTop(44);
    input_.setBounds(bounds.removeFromTop(72));
    bounds.removeFromTop(10);

    const int h = 28;
    chords_.setBounds(bounds.removeFromTop(h));
    bounds.removeFromTop(6);
    bassline_.setBounds(bounds.removeFromTop(h));
    bounds.removeFromTop(6);
    pad_.setBounds(bounds.removeFromTop(h));
    bounds.removeFromTop(6);
    arrangement_.setBounds(bounds.removeFromTop(h));
}

} // namespace sonora
