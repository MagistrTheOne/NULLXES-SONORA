#include "app/MainWindow.h"
#include "app/Version.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

class SonoraApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "SONORA"; }
    const juce::String getApplicationVersion() override { return sonora::kVersion; }
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
