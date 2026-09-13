#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"
#include "ui/components/SoniFace.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class SoniPanel : public juce::Component,
                  private juce::ChangeListener,
                  private juce::TextEditor::Listener
{
public:
    explicit SoniPanel(AppState& state);
    ~SoniPanel() override;

    bool isEditing() const;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void send();

    AppState& state_;
    SoniFace face_;
    juce::TextEditor input_;
    ActionButton send_;
};

} // namespace sonora
