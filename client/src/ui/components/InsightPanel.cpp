#include "ui/components/InsightPanel.h"
#include "ui/theme/Theme.h"

namespace sonora
{

InsightPanel::InsightPanel()
{
    generate_.setLabel("GENERATE ENGINEERING REPORT");
    generate_.onClick = [this] {
        if (onGenerateReport)
            onGenerateReport();
    };
    addAndMakeVisible(generate_);
}

void InsightPanel::setInsights(const std::vector<models::Insight>& insights)
{
    insights_ = insights;
    repaint();
}

void InsightPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(16, 14);
    theme::drawSectionLabel(g, bounds.removeFromTop(16), "ENGINEERING REPORT");
    bounds.removeFromTop(10);
    bounds.removeFromBottom(40);

    if (insights_.empty())
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawFittedText(
            "Analyze complete. Generate an engineering report.",
            bounds.removeFromTop(40),
            juce::Justification::topLeft,
            2);
        return;
    }

    const int blockH = juce::jmax(88, bounds.getHeight() / juce::jmax(1, (int) insights_.size()));
    for (const auto& item : insights_)
    {
        auto block = bounds.removeFromTop(blockH).reduced(0, 4);
        auto drawField = [&g, &block](const juce::String& k, const juce::String& v) {
            auto line = block.removeFromTop(20);
            g.setColour(colors::mutedForeground());
            g.setFont(type::label(10.0f));
            g.drawFittedText(k, line.removeFromLeft(64), juce::Justification::centredLeft, 1);
            g.setColour(colors::foreground());
            g.setFont(type::body(12.0f));
            g.drawFittedText(v, line, juce::Justification::centredLeft, 1);
        };
        drawField("ISSUE", item.issue);
        drawField("REASON", item.reason);
        drawField("ACTION", item.action);
        g.setColour(colors::border());
        g.fillRect(block.removeFromBottom(1));
    }
}

void InsightPanel::resized()
{
    generate_.setBounds(getLocalBounds().reduced(16, 14).removeFromBottom(32));
}

} // namespace sonora
