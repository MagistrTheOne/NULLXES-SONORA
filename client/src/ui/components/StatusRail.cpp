#include "ui/components/StatusRail.h"
#include "ui/theme/Theme.h"

namespace sonora
{

static juce::String computeLabel(ComputeState state)
{
    switch (state)
    {
        case ComputeState::Ready:
            return "READY";
        case ComputeState::Analyzing:
            return "ANALYZING";
        case ComputeState::Complete:
            return "COMPLETE";
        case ComputeState::Offline:
        default:
            return "OFFLINE";
    }
}

StatusRail::StatusRail()
{
    setOpaque(true);
}

void StatusRail::setState(ComputeState state)
{
    state_ = state;
    if (state == ComputeState::Complete)
        result_ = "COMPLETE";
    else if (state == ComputeState::Analyzing)
        result_ = "PENDING";
    else
        result_ = "---";
    usage_ = state == ComputeState::Offline ? "---" : "LOCAL";
    repaint();
}

void StatusRail::setUsage(const juce::String& usage)
{
    usage_ = usage;
    repaint();
}

void StatusRail::setResult(const juce::String& result)
{
    result_ = result;
    repaint();
}

void StatusRail::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());
    g.setColour(colors::border());
    g.fillRect(0, 0, getWidth(), 1);

    auto bounds = getLocalBounds().reduced(20, 0);
    const int cell = bounds.getWidth() / 3;

    auto paintCell = [&](juce::Rectangle<int> area, const juce::String& k, const juce::String& v) {
        g.setColour(colors::mutedForeground());
        g.setFont(type::label(9.0f));
        g.drawFittedText(k, area.removeFromLeft(72), juce::Justification::centredLeft, 1);
        g.setColour(colors::foreground());
        g.setFont(type::label(10.0f));
        g.drawFittedText(v, area, juce::Justification::centredLeft, 1);
    };

    paintCell(bounds.removeFromLeft(cell), "COMPUTE", computeLabel(state_));
    paintCell(bounds.removeFromLeft(cell), "USAGE", usage_);
    paintCell(bounds, "RESULT", result_);
}

} // namespace sonora
