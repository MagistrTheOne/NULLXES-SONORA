#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class SoniFace : public juce::Component
{
public:
    enum class Mode
    {
        Meet,
        Live,
        Chat
    };

    SoniFace();
    void setMode(Mode mode);
    void paint(juce::Graphics& g) override;

    static juce::Image portrait();

private:
    Mode mode_ { Mode::Chat };
};

} // namespace sonora
