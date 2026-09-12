#include "ui/components/WaveformStrip.h"

#include "ui/theme/Theme.h"

namespace sonora
{

WaveformStrip::WaveformStrip(AppState& state) : state_(state) {}

void WaveformStrip::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 10);
    auto header = bounds.removeFromTop(18);
    Theme::drawMuted(g, header.removeFromLeft(18), ">");
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawText(
        state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK",
        header.removeFromLeft(juce::jmax(80, header.getWidth() - 48)),
        juce::Justification::centredLeft,
        true);
    Theme::drawMuted(g, header, state_.durationLabel());
    bounds.removeFromTop(6);

    const auto* dna = state_.dna();
    if (dna == nullptr || dna->energyCurve.empty())
    {
        g.setColour(colors::border());
        g.fillRect(bounds);
        return;
    }

    juce::Path path;
    const float mid = (float) bounds.getCentreY();
    const float amp = (float) bounds.getHeight() * 0.46f;
    const float n = (float) dna->energyCurve.size();
    for (int i = 0; i < (int) dna->energyCurve.size(); ++i)
    {
        const float x = (float) bounds.getX() + (float) bounds.getWidth() * ((float) i / juce::jmax(1.0f, n - 1.0f));
        const float v = juce::jlimit(0.04f, 1.0f, dna->energyCurve[(size_t) i]);
        const float y = mid - v * amp;
        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }
    for (int i = (int) dna->energyCurve.size() - 1; i >= 0; --i)
    {
        const float x = (float) bounds.getX() + (float) bounds.getWidth() * ((float) i / juce::jmax(1.0f, n - 1.0f));
        const float v = juce::jlimit(0.04f, 1.0f, dna->energyCurve[(size_t) i]);
        path.lineTo(x, mid + v * amp);
    }
    path.closeSubPath();
    g.setColour(colors::foreground().withAlpha(0.22f));
    g.fillPath(path);
    g.setColour(colors::foreground().withAlpha(0.85f));
    g.strokePath(path, juce::PathStrokeType(1.1f));
}

} // namespace sonora
