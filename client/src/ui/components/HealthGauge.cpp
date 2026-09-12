#include "ui/components/HealthGauge.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

HealthGauge::HealthGauge(AppState& state) : state_(state) {}

void HealthGauge::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 14);
    Theme::drawMuted(g, bounds.removeFromTop(12), "OVERALL HEALTH");
    bounds.removeFromTop(10);

    const bool ready = state_.analysisState() == AnalysisState::Complete;
    const int score = ready ? state_.healthScore() : 0;
    const auto box = bounds.removeFromLeft(juce::jmin(110, bounds.getWidth() / 2));
    const auto centre = box.getCentre().toFloat();
    const float radius = (float) juce::jmin(box.getWidth(), box.getHeight()) * 0.42f;
    juce::Path back;
    back.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, juce::MathConstants<float>::pi * 0.75f,
                       juce::MathConstants<float>::pi * 2.25f, true);
    g.setColour(colors::border());
    g.strokePath(back, juce::PathStrokeType(4.0f));
    if (ready)
    {
        juce::Path arc;
        const float t = (float) score / 100.0f;
        arc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, juce::MathConstants<float>::pi * 0.75f,
                          juce::MathConstants<float>::pi * 0.75f + juce::MathConstants<float>::pi * 1.5f * t, true);
        g.setColour(score >= 80 ? colors::foreground() : (score >= 55 ? colors::warning() : colors::destructive()));
        g.strokePath(arc, juce::PathStrokeType(4.0f));
    }
    g.setColour(colors::foreground());
    g.setFont(type::display(18.0f));
    g.drawText(ready ? juce::String(score) : "--", box, juce::Justification::centred, true);

    auto text = bounds.reduced(8, 8);
    g.setColour(colors::foreground());
    g.setFont(type::body(14.0f));
    g.drawText(ready ? state_.healthVerdict() : "Waiting", text.removeFromTop(20), juce::Justification::centredLeft, true);
    text.removeFromTop(6);
    g.setColour(colors::mutedForeground());
    g.setFont(type::label(10.0f));
    g.drawMultiLineText(
        ready ? "From mix risks and detected collisions. Not a taste score."
              : "Load a track. SONORA will listen.",
        text.getX(),
        text.getY() + 12,
        text.getWidth());
}

} // namespace sonora
