#pragma once

#include "models/Insight.h"
#include "ui/components/ActionButton.h"

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace sonora
{

class InsightPanel : public juce::Component
{
public:
    InsightPanel();

    void setInsights(const std::vector<models::Insight>& insights);
    std::function<void()> onGenerateReport;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    std::vector<models::Insight> insights_;
    ActionButton generate_;
};

} // namespace sonora
