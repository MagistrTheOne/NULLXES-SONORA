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

juce::Colour nodeColour(NodeState state, bool selected)
{
    if (selected)
        return colors::foreground();
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
                const juce::String& lineA, NodeState state, bool selected)
{
    const bool live = state == NodeState::Loaded || state == NodeState::Processing || state == NodeState::Ready;
    g.setColour(live ? Theme::card() : Theme::surface());
    g.fillRect(box);
    g.setColour(nodeColour(state, selected));
    g.drawRect(box, selected ? 2 : 1);

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

juce::Rectangle<int> ProjectCanvas::nodeBounds(int index) const
{
    auto bounds = getLocalBounds().reduced(18, 12);
    bounds.removeFromTop(22);
    const int gap = 10;
    const int w = (bounds.getWidth() - gap * 5) / 6;
    return juce::Rectangle<int>(bounds.getX() + index * (w + gap), bounds.getY(), w, bounds.getHeight());
}

void ProjectCanvas::mouseDown(const juce::MouseEvent& event)
{
    const CanvasNode nodes[] = {
        CanvasNode::Input,
        CanvasNode::Dna,
        CanvasNode::Structure,
        CanvasNode::Mix,
        CanvasNode::Harmony,
        CanvasNode::Export
    };
    for (int i = 0; i < 6; ++i)
    {
        if (nodeBounds(i).contains(event.getPosition()))
        {
            state_.selectCanvasNode(nodes[i]);
            return;
        }
    }
}

void ProjectCanvas::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto title = getLocalBounds().reduced(18, 12);
    Theme::drawLabel(g, title.removeFromTop(12), "PROJECT CANVAS");

    const auto stage = state_.analysisState();
    const bool hasHarmony = state_.harmony().has_value();
    const auto* dna = state_.dna();
    const auto selected = state_.selectedNode();

    NodeState input = state_.hasTrack() ? NodeState::Loaded : NodeState::Idle;
    NodeState analyzer = NodeState::Idle;
    if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
        analyzer = NodeState::Processing;
    else if (stage == AnalysisState::Complete)
        analyzer = NodeState::Ready;
    else if (stage == AnalysisState::Failed)
        analyzer = NodeState::Failed;

    NodeState structure = stage == AnalysisState::Complete && dna != nullptr ? NodeState::Ready : NodeState::Waiting;
    NodeState mix = structure;
    NodeState harmony = NodeState::Waiting;
    if (hasHarmony)
        harmony = NodeState::Ready;
    else if (stage != AnalysisState::Complete)
        harmony = NodeState::Idle;

    const auto exportState = (hasHarmony || (dna != nullptr && !dna->objects.empty()))
        ? NodeState::Ready
        : NodeState::Idle;

    juce::String dnaLine = "DSP";
    if (stage == AnalysisState::Complete && dna != nullptr)
        dnaLine = state_.bpmLabel() + " / " + juce::String((int) dna->sections.size()) + " SEC";
    else if (stage == AnalysisState::Complete)
        dnaLine = state_.bpmLabel() + " BPM";

    juce::String structureLine = "map";
    if (dna != nullptr && !dna->sections.empty())
        structureLine = juce::String(dna->sections.front().name).toUpperCase();

    juce::String mixLine = "character";
    if (dna != nullptr)
        mixLine = juce::String(dna->lowEnd.risk).toUpperCase();

    const auto inputBox = nodeBounds(0);
    const auto dnaBox = nodeBounds(1);
    const auto structureBox = nodeBounds(2);
    const auto mixBox = nodeBounds(3);
    const auto harmonyBox = nodeBounds(4);
    const auto exportBox = nodeBounds(5);

    drawModule(g, inputBox, "INPUT",
               state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK",
               input, selected == CanvasNode::Input);
    drawModule(g, dnaBox, "DNA ANALYZER", dnaLine, analyzer, selected == CanvasNode::Dna);
    drawModule(g, structureBox, "STRUCTURE", structureLine, structure, selected == CanvasNode::Structure);
    drawModule(g, mixBox, "MIX ENGINE", mixLine, mix, selected == CanvasNode::Mix);
    drawModule(g, harmonyBox, "HARMONY",
               hasHarmony ? juce::String(state_.harmony()->key) : "MIDI",
               harmony, selected == CanvasNode::Harmony);
    drawModule(g, exportBox, "MIDI EXPORT", "objects", exportState, selected == CanvasNode::Export);

    drawLink(g, inputBox, dnaBox);
    drawLink(g, dnaBox, structureBox);
    drawLink(g, structureBox, mixBox);
    drawLink(g, mixBox, harmonyBox);
    drawLink(g, harmonyBox, exportBox);
}

} // namespace sonora
