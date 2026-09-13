#include "ui/components/IssueRow.h"

#include "ui/copy/HumanCopy.h"
#include "ui/theme/Theme.h"

namespace sonora
{

IssueRow::IssueRow()
{
    setOpaque(false);
}

void IssueRow::setIssue(const models::Issue& issue)
{
    empty_ = false;
    title_ = copy::issuePhrase(issue);
    detail_ = issue.detail;
    severity_ = issue.severity;
    repaint();
}

void IssueRow::setEmpty(const juce::String& message)
{
    empty_ = true;
    title_ = message;
    detail_.clear();
    severity_ = 0.0f;
    repaint();
}

void IssueRow::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(colors::border());
    g.drawLine(
        (float) bounds.getX(),
        (float) bounds.getBottom() - 0.5f,
        (float) bounds.getRight(),
        (float) bounds.getBottom() - 0.5f,
        1.0f);

    if (empty_)
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawText(title_, bounds.reduced(4, 0), juce::Justification::centredLeft, true);
        return;
    }

    auto row = bounds.reduced(4, 8);
    auto marker = row.removeFromLeft(10).withSizeKeepingCentre(6, 6);
    g.setColour(theme::severityColour(severity_));
    g.fillRect(marker);

    row.removeFromLeft(10);
    auto meter = row.removeFromRight(120);
    auto text = row;

    g.setColour(colors::foreground());
    g.setFont(type::label(11.0f));
    g.drawText(title_, text.removeFromTop(16), juce::Justification::centredLeft, true);
    g.setColour(colors::muted());
    g.setFont(type::body(11.0f));
    g.drawText(detail_, text, juce::Justification::centredLeft, true);

    g.setColour(colors::muted());
    g.setFont(type::mono(11.0f));
    auto value = meter.removeFromRight(42);
    g.drawText(juce::String(severity_, 2), value, juce::Justification::centredRight, true);

    auto track = meter.withSizeKeepingCentre(meter.getWidth() - 8, 3);
    g.setColour(colors::border());
    g.fillRect(track);
    g.setColour(theme::severityColour(severity_));
    g.fillRect(track.withWidth(juce::roundToInt((float) track.getWidth() * juce::jlimit(0.0f, 1.0f, severity_))));
}

} // namespace sonora
