#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"
#include "ui/components/ContextRail.h"
#include "ui/components/CreatePanel.h"
#include "ui/components/DnaView.h"
#include "ui/components/InsightPanel.h"
#include "ui/components/IssueRow.h"
#include "ui/components/MetricCard.h"
#include "ui/components/ProjectCanvas.h"
#include "ui/components/SpectrumView.h"
#include "ui/components/StatusRail.h"
#include "ui/components/StructureView.h"
#include "ui/components/TabBar.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>

namespace sonora
{

class Dashboard : public juce::Component,
                  private juce::ChangeListener
{
public:
    explicit Dashboard(AppState& state);
    ~Dashboard() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void refreshFromState();
    void chooseTrack();

    AppState& state_;
    ContextRail context_;
    TabBar tabs_;
    ActionButton loadTrack_;
    MetricCard bpm_;
    MetricCard key_;
    MetricCard loudness_;
    SpectrumView spectrum_;
    DnaView dnaView_;
    StructureView structureView_;
    std::array<IssueRow, 3> issues_;
    InsightPanel insights_;
    CreatePanel create_;
    ProjectCanvas canvas_;
    StatusRail status_;
    std::unique_ptr<juce::FileChooser> chooser_;
};

} // namespace sonora
