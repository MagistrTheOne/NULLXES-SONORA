#include "ui/components/SoniMeet.h"

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
    auto left = bounds.removeFromLeft(juce::jmax(280, bounds.getWidth() * 42 / 100));
    bounds.removeFromLeft(24);

    Theme::drawMuted(g, left.removeFromTop(14), "NULLXES SONORA V1.0.2");
    left.removeFromTop(8);
    g.setColour(colors::foreground());
    g.setFont(type::display(42.0f));
    g.drawText("SONI", left.removeFromTop(48), juce::Justification::centredLeft, true);
    Theme::drawMuted(g, left.removeFromTop(16), "AI ASSISTANT FOR MUSIC CREATORS");
    left.removeFromTop(16);
    Theme::drawBody(g, left.removeFromTop(20), "Understands your sound.");
    Theme::drawBody(g, left.removeFromTop(20), "Keeps you honest.");
    left.removeFromTop(20);

    juce::String quote = "не сири. не саппорт. продюсер рядом.";
    const auto& lines = state_.soniMessages();
    if (!lines.empty() && lines.front().fromSoni)
        quote = lines.front().text;
    g.setColour(colors::foreground());
    g.setFont(type::body(16.0f));
    g.drawMultiLineText(quote, left.getX(), left.getY() + 18, left.getWidth());

    auto footer = getLocalBounds().reduced(28, 20).removeFromBottom(36);
    Theme::drawMuted(g, footer.removeFromLeft(footer.getWidth() / 2),
                     "FREE VST3  ·  listen / understand / create");
    Theme::drawMuted(g, footer, "PREMIUM  ·  voice  ·  chat");
}

void SoniMeet::resized()
{
    auto bounds = getLocalBounds().reduced(28, 24);
    auto left = bounds.removeFromLeft(juce::jmax(280, bounds.getWidth() * 42 / 100));
    bounds.removeFromLeft(24);
    face_.setBounds(bounds.removeFromTop(bounds.getHeight() - 56));
    enter_.setBounds(left.removeFromBottom(40).removeFromLeft(160));
}

} // namespace sonora
