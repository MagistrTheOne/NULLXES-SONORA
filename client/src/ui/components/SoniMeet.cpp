#include "ui/components/SoniMeet.h"

#include "app/Version.h"
#include "ui/theme/Theme.h"

namespace sonora
{

SoniMeet::SoniMeet(AppState& state)
    : state_(state)
{
    face_.setMode(SoniFace::Mode::Meet);
    addAndMakeVisible(face_);
    enter_.setLabel(state_.dawHost() ? "PRESS PLAY" : "I HEAR YOU");
    enter_.onClick = [this] { state_.dismissSoniMeet(); };
    addAndMakeVisible(enter_);
    state_.addChangeListener(this);
}

SoniMeet::~SoniMeet()
{
    state_.removeChangeListener(this);
}

void SoniMeet::changeListenerCallback(juce::ChangeBroadcaster*)
{
    enter_.setLabel(state_.dawHost() ? "PRESS PLAY" : "I HEAR YOU");
    repaint();
}

void SoniMeet::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());
    auto bounds = getLocalBounds().reduced(28, 24);
    const int photo = juce::jmin(bounds.getWidth() / 2, bounds.getHeight());
    auto left = bounds.withTrimmedRight(photo + 20);

    Theme::drawMuted(g, left.removeFromTop(14), "NULLXES SONORA " + juce::String(kVersionLabel));
    left.removeFromTop(8);
    g.setColour(colors::foreground());
    g.setFont(type::display(42.0f));
    g.drawText("SONI", left.removeFromTop(48), juce::Justification::centredLeft, true);
    Theme::drawMuted(g, left.removeFromTop(16), "PRODUCER NEXT TO YOU");
    left.removeFromTop(16);
    Theme::drawBody(g, left.removeFromTop(20), "Understands your sound.");
    Theme::drawBody(g, left.removeFromTop(20), "Keeps you honest.");

    auto footer = getLocalBounds().reduced(28, 20).removeFromBottom(36);
    Theme::drawMuted(g, footer.removeFromLeft(footer.getWidth() / 2),
                     "FREE VST3  ·  listen / understand / create");
    Theme::drawMuted(g, footer, "PREMIUM  ·  voice  ·  chat");
}

void SoniMeet::resized()
{
    auto bounds = getLocalBounds();
    const int side = juce::jmin(bounds.getWidth() / 2, bounds.getHeight());
    face_.setBounds(bounds.removeFromRight(side));
    auto left = getLocalBounds().reduced(28, 24).withTrimmedRight(side + 20);
    enter_.setBounds(left.removeFromBottom(40).removeFromLeft(160));
}

} // namespace sonora
