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
            return "listening";
        case NodeState::Waiting:
            return "waiting";
        case NodeState::Ready:
            return "ready";
        case NodeState::Failed:
            return "failed";
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
    inner.removeFromTop(6);
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
    const int w = (bounds.getWidth() - gap * 4) / 5;
    return juce::Rectangle<int>(bounds.getX() + index * (w + gap), bounds.getY(), w, bounds.getHeight());
}

void ProjectCanvas::mouseDown(const juce::MouseEvent& event)
{
    const CanvasNode nodes[] = {
        CanvasNode::Track,
        CanvasNode::Understand,
        CanvasNode::Improve,
        CanvasNode::Create,
        CanvasNode::Export
    };
    for (int i = 0; i < 5; ++i)
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
    const auto* dna = state_.dna();
    const auto selected = state_.selectedNode();
    const bool ready = stage == AnalysisState::Complete;

    NodeState track = state_.hasTrack() ? NodeState::Loaded : NodeState::Idle;
    NodeState understand = NodeState::Idle;
    if (stage == AnalysisState::Loading || stage == AnalysisState::Analyzing)
        understand = NodeState::Processing;
    else if (ready)
        understand = NodeState::Ready;
    else if (stage == AnalysisState::Failed)
        understand = NodeState::Failed;

    NodeState improve = ready ? NodeState::Ready : NodeState::Waiting;
    NodeState create = state_.harmony() || state_.eqProfile() ? NodeState::Ready
                       : (ready ? NodeState::Waiting : NodeState::Idle);
    NodeState exportNode = (dna != nullptr && !dna->objects.empty()) || state_.harmony()
        ? NodeState::Ready
        : NodeState::Idle;

    const auto a = nodeBounds(0);
    const auto b = nodeBounds(1);
    const auto c = nodeBounds(2);
    const auto d = nodeBounds(3);
    const auto e = nodeBounds(4);

    drawModule(g, a, "TRACK",
               state_.hasTrack() ? juce::String(state_.loadedFilename()) : "NO TRACK",
               track, selected == CanvasNode::Track);
    drawModule(g, b, "UNDERSTAND",
               ready ? state_.styleLabel() : "listen",
               understand, selected == CanvasNode::Understand);
    drawModule(g, c, "IMPROVE",
               ready ? juce::String((int) state_.mixRows().size()) + " checks" : "mix",
               improve, selected == CanvasNode::Improve);
    drawModule(g, d, "CREATE",
               state_.harmony() ? "MIDI ready" : "objects",
               create, selected == CanvasNode::Create);
    drawModule(g, e, "EXPORT",
               ready ? state_.objectCountLabel() : "MIDI / stems",
               exportNode, selected == CanvasNode::Export);

    drawLink(g, a, b);
    drawLink(g, b, c);
    drawLink(g, c, d);
    drawLink(g, d, e);
}

} // namespace sonora
