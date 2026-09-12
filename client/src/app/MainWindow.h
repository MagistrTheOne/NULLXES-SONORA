#pragma once

#include "state/AppState.h"
#include "ui/screens/Dashboard.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace sonora
{

class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);

    void closeButtonPressed() override;

private:
    AppState state_;
    Dashboard dashboard_ { state_ };
};

} // namespace sonora
