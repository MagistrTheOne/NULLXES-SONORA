#include "app/MainWindow.h"
#include "backend/ClientLog.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

class SonoraApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "SONORA"; }
    const juce::String getApplicationVersion() override { return "0.2.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                       .getChildFile("NULLXES")
                       .getChildFile("SONORA");
        dir.createDirectory();
        logger_ = std::make_unique<juce::FileLogger>(dir.getChildFile("client.log"), "SONORA client");
        juce::Logger::setCurrentLogger(logger_.get());
        sonora::clientLog("SONORA start");
        mainWindow_ = std::make_unique<sonora::MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        sonora::clientLog("SONORA shutdown");
        mainWindow_.reset();
        juce::Logger::setCurrentLogger(nullptr);
        logger_.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<juce::FileLogger> logger_;
    std::unique_ptr<sonora::MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(SonoraApplication)
