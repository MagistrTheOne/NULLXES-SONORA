#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"
#include "ui/components/AssistantPanel.h"
#include "ui/components/InsightPanel.h"
#include "ui/components/IssueRow.h"
#include "ui/components/MetricCard.h"
#include "ui/components/SpectrumView.h"
#include "ui/components/StatusRail.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace sonora
{

class Dashboard : public juce::Component
{
public:
    explicit Dashboard(AppState& state);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void refreshFromState();

    AppState& state_;
    ActionButton loadTrack_;
    MetricCard bpm_;
    MetricCard key_;
    MetricCard loudness_;
    SpectrumView spectrum_;
    std::array<IssueRow, 3> issues_;
    InsightPanel insights_;
    AssistantPanel assistant_;
    StatusRail status_;
};

} // namespace sonora
