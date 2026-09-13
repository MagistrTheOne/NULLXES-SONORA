#include "ui/components/LiveListenView.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

#include <cmath>

namespace sonora
{

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
    const bool waiting = !state_.hostPlaying() && state_.analysisState() != AnalysisState::Complete;
    g.drawText(waiting ? "WAITING FOR PLAYBACK" : "NOW PLAYING",
               bounds.removeFromTop(28), juce::Justification::centredLeft, true);
    bounds.removeFromTop(6);
    Theme::drawBody(g, bounds.removeFromTop(20),
                    waiting ? "Press Play in FL Studio"
                            : (state_.nowSection() + "    " + state_.barLabel() + "    "
                               + copy::formatTime((float) state_.hostSeconds())));
    bounds.removeFromTop(10);
    Theme::drawMuted(g, bounds.removeFromTop(14), "TEMPO");
    g.setColour(colors::foreground());
    g.setFont(type::display(28.0f));
    g.drawText(state_.bpmLabel() + " BPM", bounds.removeFromTop(34), juce::Justification::centredLeft, true);
    bounds.removeFromTop(10);

    Theme::drawMuted(g, bounds.removeFromTop(14), "ENERGY");
    bounds.removeFromTop(6);
    auto meter = bounds.removeFromTop(16);
    g.setColour(colors::border());
    g.fillRect(meter);
    const float energy = state_.energyNow();
    const float glow = 0.4f + 0.6f * (0.5f + 0.5f * std::sin(pulse_));
    g.setColour(colors::foreground().withAlpha(0.35f + 0.55f * glow * juce::jmax(0.15f, energy)));
    g.fillRect(meter.withWidth(juce::jmax(3, juce::roundToInt((float) meter.getWidth() * energy))));
    bounds.removeFromTop(8);
    Theme::drawBody(g, bounds.removeFromTop(18), state_.energyLabel());

    bounds.removeFromTop(12);
    Theme::drawMuted(g, bounds.removeFromTop(14), "MIX HEALTH");
    bounds.removeFromTop(4);
    Theme::drawBody(g, bounds.removeFromTop(18),
                    state_.analysisState() == AnalysisState::Complete
                        ? juce::String(state_.healthScore()) + "%   " + state_.healthVerdict()
                        : (state_.hostPlaying() ? "Mapping the session..." : "SONORA is on the bus."));

    bounds.removeFromTop(14);
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
