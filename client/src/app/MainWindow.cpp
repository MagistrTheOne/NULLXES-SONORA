#include "app/MainWindow.h"
#include "ui/Theme.h"

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
    setResizeLimits(960, 640, 4096, 2160);
    centreWithSize(1280, 800);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace sonora
