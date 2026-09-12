#include "ui/components/WaveformStrip.h"

#include "ui/theme/Theme.h"

namespace sonora
{

WaveformStrip::WaveformStrip(AppState& state) : state_(state)
{
    startTimerHz(30);
}

WaveformStrip::~WaveformStrip()
{
    stopTimer();
}

juce::Rectangle<int> WaveformStrip::playBounds() const
{
    return getLocalBounds().reduced(16, 10).withHeight(18).withWidth(22);
}

juce::Rectangle<int> WaveformStrip::waveBounds() const
{
    auto bounds = getLocalBounds().reduced(16, 10);
    bounds.removeFromTop(24);
    return bounds;
}

void WaveformStrip::timerCallback()
{
    if (state_.isPlaying())
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
    auto bounds = getLocalBounds().reduced(16, 10);
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
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawText(
        state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK",
        header.removeFromLeft(juce::jmax(80, header.getWidth() - 72)),
        juce::Justification::centredLeft,
        true);
    Theme::drawMuted(g, header, state_.playheadLabel());
    bounds.removeFromTop(6);

    const auto* dna = state_.dna();
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
        g.fillRect(bounds);
        return;
    }

    const float mid = (float) bounds.getCentreY();
    const float amp = (float) bounds.getHeight() * 0.46f;
    const int n = (int) samples->size();
    const float barW = juce::jmax(1.0f, (float) bounds.getWidth() / (float) juce::jmax(1, n));
    for (int i = 0; i < n; ++i)
    {
        const float v = juce::jlimit(0.03f, 1.0f, (*samples)[(size_t) i]);
        const float h = v * amp;
        const float x = (float) bounds.getX() + (float) i * barW;
        g.setColour(colors::foreground().withAlpha(0.55f));
        g.fillRect(x, mid - h, juce::jmax(1.0f, barW - 0.4f), h * 2.0f);
    }

    if (state_.canPlay() || state_.isPlaying())
    {
        const float x = (float) bounds.getX() + (float) bounds.getWidth() * state_.playhead();
        g.setColour(colors::foreground());
        g.fillRect(x, (float) bounds.getY(), 1.4f, (float) bounds.getHeight());
    }
}

} // namespace sonora
