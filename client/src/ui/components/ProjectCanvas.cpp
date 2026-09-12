#include "ui/components/ProjectCanvas.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
enum class NodeState
{
    Idle,
    Waiting,
    Loaded,
    Processing,
    Ready,
    Failed
};

juce::Colour nodeColour(NodeState state)
{
    switch (state)
    {
        case NodeState::Processing:
            return colors::warning();
        case NodeState::Failed:
            return colors::destructive();
        case NodeState::Loaded:
        case NodeState::Ready:
            return colors::borderStrong();
        case NodeState::Waiting:
        case NodeState::Idle:
        default:
            return colors::border();
    }
}

juce::String nodeLabel(NodeState state)
{
    switch (state)
    {
        case NodeState::Loaded:
            return "loaded";
        case NodeState::Processing:
            return "processing";
        case NodeState::Waiting:
            return "waiting";
        case NodeState::Ready:
            return "ready";
        case NodeState::Failed:
            return "failed";
        case NodeState::Idle:
        default:
            return "idle";
    }
}

void drawModule(juce::Graphics& g, juce::Rectangle<int> box, const juce::String& title,
                const juce::String& lineA, NodeState state)
{
    const bool live = state == NodeState::Loaded || state == NodeState::Processing || state == NodeState::Ready;
    g.setColour(live ? Theme::card() : Theme::surface());
    g.fillRect(box);
    g.setColour(nodeColour(state));
    g.drawRect(box, 1);

    auto inner = box.reduced(10, 8);
    Theme::drawLabel(g, inner.removeFromTop(12), title);
    inner.removeFromTop(8);
    Theme::drawBody(g, inner.removeFromTop(16), lineA);
    Theme::drawMuted(g, inner.removeFromTop(14), nodeLabel(state));
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
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(18, 12);
    Theme::drawLabel(g, bounds.removeFromTop(12), "PROJECT CANVAS");
    bounds.removeFromTop(10);

    const int gap = 10;
    const int w = (bounds.getWidth() - gap * 5) / 6;
    auto slot = [&](int i) {
        return juce::Rectangle<int>(bounds.getX() + i * (w + gap), bounds.getY(), w, bounds.getHeight());
    };

    const auto stage = state_.analysisState();
    const bool hasHarmony = state_.harmony().has_value();

    NodeState input = state_.hasTrack() ? NodeState::Loaded : NodeState::Idle;
    NodeState analyzer = NodeState::Idle;
    if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
        analyzer = NodeState::Processing;
    else if (stage == AnalysisState::Complete)
        analyzer = NodeState::Ready;
    else if (stage == AnalysisState::Failed)
        analyzer = NodeState::Failed;

    NodeState harmony = NodeState::Waiting;
    if (hasHarmony)
        harmony = NodeState::Ready;
    else if (stage != AnalysisState::Complete)
        harmony = NodeState::Idle;

    const auto exportState = hasHarmony ? NodeState::Ready : NodeState::Idle;

    const auto inputBox = slot(0);
    const auto analyzerBox = slot(1);
    const auto harmonyBox = slot(2);
    const auto bassBox = slot(3);
    const auto padBox = slot(4);
    const auto exportBox = slot(5);

    drawModule(g, inputBox, "INPUT",
               state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK", input);
    drawModule(g, analyzerBox, "ANALYZER",
               stage == AnalysisState::Complete ? state_.bpmLabel() + " BPM" : "DSP", analyzer);
    drawModule(g, harmonyBox, "HARMONY",
               hasHarmony ? juce::String(state_.harmony()->key) : "MIDI", harmony);
    drawModule(g, bassBox, "BASS ENGINE", "pattern", NodeState::Waiting);
    drawModule(g, padBox, "PAD GENERATOR", "layer", NodeState::Waiting);
    drawModule(g, exportBox, "EXPORT MIDI", "clip", exportState);

    drawLink(g, inputBox, analyzerBox);
    drawLink(g, analyzerBox, harmonyBox);
    drawLink(g, harmonyBox, bassBox);
    drawLink(g, bassBox, padBox);
    drawLink(g, padBox, exportBox);
}

} // namespace sonora
