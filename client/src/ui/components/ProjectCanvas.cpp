#include "ui/components/ProjectCanvas.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
void drawModule(juce::Graphics& g, juce::Rectangle<int> box, const juce::String& title,
                const juce::String& lineA, const juce::String& lineB, bool active)
{
    g.setColour(active ? Theme::card() : Theme::surface());
    g.fillRoundedRectangle(box.toFloat(), 8.0f);
    g.setColour(active ? Theme::accent().withAlpha(0.55f) : Theme::border());
    g.drawRoundedRectangle(box.toFloat().reduced(0.5f), 8.0f, 1.0f);

    auto inner = box.reduced(10, 8);
    Theme::drawLabel(g, inner.removeFromTop(12), title);
    inner.removeFromTop(8);
    Theme::drawBody(g, inner.removeFromTop(16), lineA);
    Theme::drawMuted(g, inner.removeFromTop(14), lineB);
}

void drawLink(juce::Graphics& g, juce::Rectangle<int> from, juce::Rectangle<int> to)
{
    const auto start = juce::Point<float>((float) from.getRight(), (float) from.getCentreY());
    const auto end = juce::Point<float>((float) to.getX(), (float) to.getCentreY());
    g.setColour(Theme::muted().withAlpha(0.45f));
    g.drawLine(start.x, start.y, end.x, end.y, 1.2f);
}
} // namespace

ProjectCanvas::ProjectCanvas(AppState& state) : state_(state) {}

void ProjectCanvas::paint(juce::Graphics& g)
{
    g.setColour(Theme::card());
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);

    auto bounds = getLocalBounds().reduced(18, 12);
    Theme::drawLabel(g, bounds.removeFromTop(12), "PROJECT CANVAS");
    bounds.removeFromTop(10);

    const int gap = 10;
    const int w = (bounds.getWidth() - gap * 5) / 6;
    auto slot = [&](int i) {
        return juce::Rectangle<int>(bounds.getX() + i * (w + gap), bounds.getY(), w, bounds.getHeight());
    };

    const bool hasTrack = state_.hasTrack();
    const bool analyzing = state_.analysisState() == AnalysisState::Analyzing
                           || state_.analysisState() == AnalysisState::Loading;
    const bool complete = state_.analysisState() == AnalysisState::Complete;
    const bool hasHarmony = state_.harmony().has_value();

    juce::String bpm = complete ? state_.bpmLabel() + " BPM" : "waiting";
    juce::String key = complete ? state_.keyLabel() : "waiting";

    const auto input = slot(0);
    const auto analyzer = slot(1);
    const auto harmony = slot(2);
    const auto bass = slot(3);
    const auto pad = slot(4);
    const auto exportMidi = slot(5);

    drawModule(g, input, "INPUT", hasTrack ? juce::String(state_.loadedFilename()) : "NO TRACK", "drag >", hasTrack);
    drawModule(g, analyzer, "ANALYZER", analyzing ? "WORKING" : (complete ? "READY" : "IDLE"), bpm, analyzing || complete);
    drawModule(g, harmony, "HARMONY", hasHarmony ? juce::String(state_.harmony()->key) : key, hasHarmony ? "MIDI clip" : "drag >", hasHarmony);
    drawModule(g, bass, "BASS ENGINE", "pending", "drag >", false);
    drawModule(g, pad, "PAD GENERATOR", "pending", "drag >", false);
    drawModule(g, exportMidi, "EXPORT MIDI", hasHarmony ? "READY" : "idle", "drag >", hasHarmony);

    drawLink(g, input, analyzer);
    drawLink(g, analyzer, harmony);
    drawLink(g, harmony, bass);
    drawLink(g, bass, pad);
    drawLink(g, pad, exportMidi);
}

} // namespace sonora
