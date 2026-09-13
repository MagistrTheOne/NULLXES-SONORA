#include "app/MainWindow.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

class SonoraApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "SONORA"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        juce::Logger::setCurrentLogger(nullptr);
        mainWindow_ = std::make_unique<sonora::MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<sonora::MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(SonoraApplication)
