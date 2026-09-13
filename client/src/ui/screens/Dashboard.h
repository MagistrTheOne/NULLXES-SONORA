#pragma once

#include "state/AppState.h"
#include "ui/components/ActionButton.h"
#include "ui/components/ActionsPanel.h"
#include "ui/components/ArrangementStrip.h"
#include "ui/components/ContextRail.h"
#include "ui/components/CreatePage.h"
#include "ui/components/CreatePanel.h"
#include "ui/components/DnaView.h"
#include "ui/components/HealthGauge.h"
#include "ui/components/IdentityRow.h"
#include "ui/components/LabOverlay.h"
#include "ui/components/LiveListenView.h"
#include "ui/components/MaskingHeatmap.h"
#include "ui/components/MixHealthView.h"
#include "ui/components/ProblemsView.h"
#include "ui/components/ProjectCanvas.h"
#include "ui/components/ReferencePage.h"
#include "ui/components/SoniMeet.h"
#include "ui/components/SoniPanel.h"
#include "ui/components/SonicIdentity.h"
#include "ui/components/SpectrumView.h"
#include "ui/components/StatusRail.h"
#include "ui/components/StructureView.h"
#include "ui/components/TopBar.h"
#include "ui/components/TranslationStrip.h"
#include "ui/components/WaveformStrip.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>

namespace sonora
{

class Dashboard : public juce::Component,
                  private juce::ChangeListener
{
public:
    explicit Dashboard(AppState& state);
    ~Dashboard() override;

    void setCaptureSite(std::function<void*()> site);

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void refreshFromState();
    void chooseTrack();
    void hideWorkspace();

    AppState& state_;
    TopBar topBar_;
    ContextRail context_;
    ActionButton loadTrack_;
    ActionButton listen_;
    ActionButton reference_;
    LiveListenView live_;
    WaveformStrip waveform_;
    IdentityRow identity_;
    SonicIdentity sonic_;
    ArrangementStrip arrangement_;
    MixHealthView mixHealth_;
    ProblemsView problems_;
    ActionsPanel actions_;
    SpectrumView spectrum_;
    TranslationStrip translation_;
    MaskingHeatmap masking_;
    HealthGauge health_;
    CreatePanel createRail_;
    CreatePage createPage_;
    ReferencePage referencePage_;
    StructureView structure_;
    DnaView dnaView_;
    ProjectCanvas canvas_;
    StatusRail status_;
    SoniPanel soni_;
    SoniMeet meet_;
    LabOverlay lab_;
    std::function<void*()> captureSite_;
    std::unique_ptr<juce::FileChooser> chooser_;
};

} // namespace sonora
