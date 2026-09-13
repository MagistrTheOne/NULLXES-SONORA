#include "ui/components/LiveListenView.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

#include <cmath>

namespace sonora
{
namespace
{
juce::String arrow(int dir)
{
    if (dir > 0)
        return juce::String::fromUTF8("\xe2\x86\x91");
    if (dir < 0)
        return juce::String::fromUTF8("\xe2\x86\x93");
    return juce::String::fromUTF8("\xe2\x86\x92");
}
} // namespace

LiveListenView::LiveListenView(AppState& state)
    : state_(state)
{
    face_.setMode(SoniFace::Mode::Live);
    addAndMakeVisible(face_);
    startTimerHz(20);
}

LiveListenView::~LiveListenView()
{
    stopTimer();
}

void LiveListenView::timerCallback()
{
    pulse_ += 0.11f;
    repaint();
}

void LiveListenView::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    face_.setBounds(bounds.removeFromRight(juce::jmin(200, bounds.getWidth() / 3)).removeFromTop(240));
}

void LiveListenView::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(20, 16);
    bounds.removeFromRight(juce::jmin(212, bounds.getWidth() / 3) + 12);

    Theme::drawMuted(g, bounds.removeFromTop(14), "LISTEN");
    bounds.removeFromTop(8);
    g.setColour(colors::foreground());
    g.setFont(type::display(22.0f));
    const auto phase = state_.listenPhase();
    g.drawText(state_.listenHeadline(), bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(6);
    Theme::drawBody(g, bounds.removeFromTop(20), state_.listenHint());
    bounds.removeFromTop(10);

    if (phase != ListenPhase::Understood)
    {
        Theme::drawMuted(g, bounds.removeFromTop(14), "PROGRESS");
        bounds.removeFromTop(6);
        auto meter = bounds.removeFromTop(16);
        g.setColour(colors::border());
        g.fillRect(meter);
        const float glow = 0.4f + 0.6f * (0.5f + 0.5f * std::sin(pulse_));
        g.setColour(colors::foreground().withAlpha(0.35f + 0.55f * glow));
        g.fillRect(meter.withWidth(juce::jmax(3, juce::roundToInt((float) meter.getWidth() * state_.listenProgress()))));
        bounds.removeFromTop(8);
        Theme::drawBody(g, bounds.removeFromTop(18),
                        phase == ListenPhase::Waiting ? "Press Play in FL Studio"
                        : phase == ListenPhase::Listening ? "Energy detected"
                                                          : "Structure mapping...");
        bounds.removeFromTop(12);
    }
    else
    {
        Theme::drawMuted(g, bounds.removeFromTop(14), "IDENTITY");
        bounds.removeFromTop(4);
        g.setColour(colors::foreground());
        g.setFont(type::display(28.0f));
        g.drawText(state_.bpmLabel() + " BPM", bounds.removeFromTop(32), juce::Justification::centredLeft, true);
        Theme::drawBody(g, bounds.removeFromTop(18),
                        state_.keyLabel() + "    " + state_.styleLabel());
        bounds.removeFromTop(8);
        Theme::drawMuted(g, bounds.removeFromTop(14), "STRUCTURE");
        Theme::drawBody(g, bounds.removeFromTop(18), state_.structureLine());
        bounds.removeFromTop(6);
        Theme::drawMuted(g, bounds.removeFromTop(14), "MIX");
        Theme::drawBody(g, bounds.removeFromTop(18), state_.mixLine());
        bounds.removeFromTop(12);
    }

    Theme::drawMuted(g, bounds.removeFromTop(14), "ENERGY");
    bounds.removeFromTop(6);
    auto energyMeter = bounds.removeFromTop(16);
    g.setColour(colors::border());
    g.fillRect(energyMeter);
    const float energy = state_.energyNow();
    const float glow = 0.4f + 0.6f * (0.5f + 0.5f * std::sin(pulse_));
    g.setColour(colors::foreground().withAlpha(0.35f + 0.55f * glow * juce::jmax(0.15f, energy)));
    g.fillRect(energyMeter.withWidth(juce::jmax(3, juce::roundToInt((float) energyMeter.getWidth() * energy))));
    bounds.removeFromTop(8);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    state_.hostPlaying() ? (state_.nowSection() + "    " + state_.barLabel())
                                         : state_.energyLabel());

    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "LIVE MIX STATE");
    bounds.removeFromTop(4);
    for (const auto& flag : state_.liveMixFlags())
        Theme::drawBody(g, bounds.removeFromTop(18), flag.name + "  " + arrow(flag.dir));

    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "SONI");
    bounds.removeFromTop(4);
    juce::String line = "жми play. я услышу.";
    const auto& messages = state_.soniMessages();
    for (int i = (int) messages.size() - 1; i >= 0; --i)
    {
        if (messages[(size_t) i].fromSoni)
        {
            line = messages[(size_t) i].text.replace("\n", "  ");
            break;
        }
    }
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawMultiLineText(line, bounds.getX(), bounds.getY() + 14, bounds.getWidth());
}

} // namespace sonora
