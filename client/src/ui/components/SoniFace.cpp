#include "ui/components/SoniFace.h"

#include "SoniBinary.h"
#include "ui/theme/Theme.h"

namespace sonora
{

juce::Image SoniFace::portrait()
{
    static juce::Image image;
    if (!image.isValid())
        image = juce::ImageFileFormat::loadFrom(SoniBinary::soni_jpg, SoniBinary::soni_jpgSize);
    return image;
}

SoniFace::SoniFace()
{
    setOpaque(false);
}

void SoniFace::setMode(Mode mode)
{
    mode_ = mode;
    repaint();
}

void SoniFace::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto image = portrait();
    if (!image.isValid())
    {
        g.setColour(colors::elevated());
        g.fillRect(bounds);
        return;
    }

    const float iw = (float) image.getWidth();
    const float ih = (float) image.getHeight();
    const float scale = juce::jmax(bounds.getWidth() / iw, bounds.getHeight() / ih);
    auto src = juce::Rectangle<float>(0, 0, iw, ih);
    auto dest = juce::Rectangle<float>(0, 0, iw * scale, ih * scale);
    dest.setCentre(bounds.getCentre());
    if (mode_ == Mode::Meet)
        dest.translate(0.0f, bounds.getHeight() * 0.04f);
    g.drawImage(image, dest);

    if (mode_ != Mode::Meet)
    {
        g.setColour(colors::background().withAlpha(0.18f));
        g.fillRect(bounds);
    }
}

} // namespace sonora
