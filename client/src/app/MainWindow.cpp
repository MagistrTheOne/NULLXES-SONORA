#include "app/MainWindow.h"
#include "ui/theme/Theme.h"

namespace sonora
{

MainWindow::MainWindow(const juce::String& name)
    : juce::DocumentWindow(
          name,
          theme::background(),
          juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setContentNonOwned(&dashboard_, true);
    setResizable(true, true);
    setResizeLimits(1280, 800, 4096, 2160);
    centreWithSize(1600, 960);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

bool MainWindow::keyPressed(const juce::KeyPress& key)
{
    return state_.handleKeyPress(key);
}

} // namespace sonora
