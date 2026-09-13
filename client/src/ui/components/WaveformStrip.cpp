#include "ui/components/WaveformStrip.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

#include <cmath>

namespace sonora
{

WaveformStrip::WaveformStrip(AppState& state) : state_(state)
{
    startTimerHz(24);
}

WaveformStrip::~WaveformStrip()
{
    stopTimer();
}

juce::Rectangle<int> WaveformStrip::playBounds() const
{
    return getLocalBounds().reduced(16, 12).removeFromTop(18).removeFromLeft(22);
}

juce::Rectangle<int> WaveformStrip::waveBounds() const
{
    auto bounds = getLocalBounds().reduced(16, 12);
    bounds.removeFromTop(40);
    bounds.removeFromBottom(36);
    return bounds;
}

void WaveformStrip::timerCallback()
{
    phase_ += 0.09f * (0.35f + state_.energyNow());
    if (phase_ > juce::MathConstants<float>::twoPi)
        phase_ -= juce::MathConstants<float>::twoPi;
    repaint();
}

void WaveformStrip::seekFrom(const juce::MouseEvent& event)
{
    const auto wave = waveBounds();
    if (wave.getWidth() <= 0)
        return;
    const float t = (float) (event.x - wave.getX()) / (float) wave.getWidth();
    state_.seekPlayhead(juce::jlimit(0.0f, 1.0f, t));
}

void WaveformStrip::mouseDown(const juce::MouseEvent& event)
{
    if (playBounds().contains(event.getPosition()))
    {
        state_.togglePlayback();
        return;
    }
    if (waveBounds().contains(event.getPosition()) && state_.canPlay())
        seekFrom(event);
}

void WaveformStrip::mouseDrag(const juce::MouseEvent& event)
{
    if (waveBounds().contains(event.getPosition()) && state_.canPlay())
        seekFrom(event);
}

void WaveformStrip::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    auto header = bounds.removeFromTop(18);
    auto play = header.removeFromLeft(22);
    g.setColour(state_.canPlay() ? colors::foreground() : colors::mutedForeground());
    if (state_.isPlaying())
    {
        g.fillRect(play.getX() + 5, play.getY() + 3, 4, 12);
        g.fillRect(play.getX() + 12, play.getY() + 3, 4, 12);
    }
    else
    {
        juce::Path triangle;
        triangle.addTriangle(
            (float) play.getX() + 6.0f,
            (float) play.getY() + 3.0f,
            (float) play.getX() + 6.0f,
            (float) play.getY() + 15.0f,
            (float) play.getX() + 17.0f,
            (float) play.getY() + 9.0f);
        g.fillPath(triangle);
    }
    header.removeFromLeft(8);
    g.setColour(colors::mutedForeground());
    g.setFont(type::mono(11.0f));
    const auto now = copy::formatTime(state_.playheadSeconds());
    g.drawText(now, header.removeFromLeft(42), juce::Justification::centredLeft, true);
    auto endTime = header.removeFromRight(42);
    g.drawText(state_.durationLabel(), endTime, juce::Justification::centredRight, true);
    header.removeFromLeft(8);
    header.removeFromRight(8);
    g.setColour(colors::border());
    g.fillRect(header.getX(), header.getCentreY(), header.getWidth(), 1);
    const float playX = (float) header.getX() + (float) header.getWidth() * state_.playhead();
    g.setColour(colors::foreground().withAlpha(0.7f));
    g.fillEllipse(playX - 2.5f, (float) header.getCentreY() - 2.5f, 5.0f, 5.0f);

    bounds.removeFromTop(8);
    auto labels = bounds.removeFromTop(14);
    const auto* dna = state_.dna();
    const float duration = state_.analysis() ? state_.analysis()->durationSec : 0.0f;
    if (dna != nullptr && duration > 0.0f)
    {
        for (const auto& section : dna->sections)
        {
            const float x0 = (float) labels.getX() + (section.start / duration) * (float) labels.getWidth();
            const float x1 = (float) labels.getX() + (section.end / duration) * (float) labels.getWidth();
            g.setColour(colors::mutedForeground());
            g.setFont(type::label(10.0f));
            g.drawText(copy::sectionLabel(section.name),
                       juce::Rectangle<int>((int) x0, labels.getY(), juce::jmax(36, (int) (x1 - x0)), labels.getHeight()),
                       juce::Justification::centredLeft,
                       true);
        }
    }

    auto pulse = bounds.removeFromBottom(22);
    bounds.removeFromBottom(6);
    auto blocks = bounds.removeFromBottom(10);
    bounds.removeFromBottom(6);
    const auto wave = bounds;

    const std::vector<float>* samples = nullptr;
    if (dna != nullptr)
    {
        if (!dna->energyPeaks.empty())
            samples = &dna->energyPeaks;
        else if (!dna->energyCurve.empty())
            samples = &dna->energyCurve;
    }

    if (samples == nullptr)
    {
        g.setColour(colors::border());
        g.fillRect(wave);
    }
    else
    {
        const float mid = (float) wave.getCentreY();
        const float amp = (float) wave.getHeight() * 0.46f;
        const int n = (int) samples->size();
        const float barW = juce::jmax(1.0f, (float) wave.getWidth() / (float) juce::jmax(1, n));
        for (int i = 0; i < n; ++i)
        {
            const float v = juce::jlimit(0.03f, 1.0f, (*samples)[(size_t) i]);
            const float h = v * amp;
            const float x = (float) wave.getX() + (float) i * barW;
            g.setColour(colors::foreground().withAlpha(0.55f));
            g.fillRect(x, mid - h, juce::jmax(1.0f, barW - 0.4f), h * 2.0f);
        }
    }

    if (dna != nullptr && duration > 0.0f)
    {
        for (const auto& section : dna->sections)
        {
            const float x0 = (float) blocks.getX() + (section.start / duration) * (float) blocks.getWidth();
            const float x1 = (float) blocks.getX() + (section.end / duration) * (float) blocks.getWidth();
            auto cell = juce::Rectangle<float>(x0, (float) blocks.getY(), juce::jmax(8.0f, x1 - x0 - 4.0f), (float) blocks.getHeight());
            Theme::drawBlocks(g, cell.toNearestInt(), section.energy, 8);
        }
    }

    const float energy = state_.energyNow();
    const float glow = 0.35f + 0.65f * (0.5f + 0.5f * std::sin(phase_));
    Theme::drawMuted(g, pulse.removeFromLeft(64), "ENERGY");
    g.setColour(colors::foreground());
    g.setFont(type::display(16.0f));
    g.drawText(state_.energyLabel(), pulse.removeFromLeft(48), juce::Justification::centredLeft, true);
    auto meter = pulse.removeFromLeft(juce::jmax(90, pulse.getWidth() - 28)).reduced(0, 6);
    g.setColour(colors::border());
    g.fillRect(meter);
    g.setColour(colors::foreground().withAlpha(0.35f + 0.55f * glow * energy));
    g.fillRect(meter.withWidth(juce::jmax(3, juce::roundToInt((float) meter.getWidth() * energy))));
    auto dot = pulse.removeFromRight(14).withSizeKeepingCentre(8, 8);
    g.setColour(colors::foreground().withAlpha(0.25f + 0.75f * glow * juce::jmax(0.2f, energy)));
    g.fillEllipse(dot.toFloat());

    if (state_.canPlay() || state_.isPlaying())
    {
        const float x = (float) wave.getX() + (float) wave.getWidth() * state_.playhead();
        g.setColour(colors::foreground());
        g.fillRect(x, (float) wave.getY(), 1.4f, (float) wave.getHeight());
    }
}

} // namespace sonora
