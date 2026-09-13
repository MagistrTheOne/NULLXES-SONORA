#include "ui/components/SoniFace.h"

#include "SoniBinary.h"
#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
juce::Rectangle<int> faceSquare(const juce::Image& image)
{
    const float iw = (float) image.getWidth();
    const float ih = (float) image.getHeight();
    const float side = juce::jmin(iw, ih) * 0.84f;
    float cx = iw * 0.57f;
    float cy = ih * 0.28f;
    auto box = juce::Rectangle<float>(side, side).withCentre({ cx, cy });
    if (box.getX() < 0.0f)
        box.setX(0.0f);
    if (box.getY() < 0.0f)
        box.setY(0.0f);
    if (box.getRight() > iw)
        box.setX(iw - side);
    if (box.getBottom() > ih)
        box.setY(ih - side);
    return box.toNearestInt();
}
} // namespace

juce::Image SoniFace::portrait()
{
    static juce::Image image;
    if (!image.isValid())
        image = juce::ImageFileFormat::loadFrom(SoniBinary::soni_jpg, SoniBinary::soni_jpgSize);
    return image;
}

SoniFace::SoniFace()
{
    setOpaque(true);
}

void SoniFace::setMode(Mode mode)
{
    mode_ = mode;
    repaint();
}

void SoniFace::paint(juce::Graphics& g)
{
    g.fillAll(colors::background());
    auto bounds = getLocalBounds();
    const auto image = portrait();
    if (!image.isValid() || bounds.isEmpty())
        return;

    const auto src = faceSquare(image);
    const float scale = juce::jmax((float) bounds.getWidth() / (float) src.getWidth(),
                                   (float) bounds.getHeight() / (float) src.getHeight());
    auto dest = juce::Rectangle<float>((float) src.getWidth() * scale, (float) src.getHeight() * scale);
    dest.setCentre(bounds.toFloat().getCentre());
    g.reduceClipRegion(bounds);
    g.drawImage(image,
                juce::roundToInt(dest.getX()), juce::roundToInt(dest.getY()),
                juce::roundToInt(dest.getWidth()), juce::roundToInt(dest.getHeight()),
                src.getX(), src.getY(), src.getWidth(), src.getHeight());
}

} // namespace sonora
