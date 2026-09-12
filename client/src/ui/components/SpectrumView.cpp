#include "ui/components/SpectrumView.h"
#include "ui/theme/Theme.h"

namespace sonora
{

SpectrumView::SpectrumView()
{
    setOpaque(false);
}

void SpectrumView::setBands(const models::FrequencyDistribution& bands)
{
    bands_ = bands;
    hasData_ = true;
    repaint();
}

void SpectrumView::clear()
{
    bands_ = {};
    hasData_ = false;
    repaint();
}

void SpectrumView::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto inner = getLocalBounds().reduced(16, 14);
    theme::drawSectionLabel(g, inner.removeFromTop(16), "SPECTRUM");
    inner.removeFromTop(8);

    const float values[] = {
        hasData_ ? bands_.sub : 0.08f,
        hasData_ ? bands_.low : 0.08f,
        hasData_ ? bands_.mid : 0.08f,
        hasData_ ? bands_.high : 0.08f,
        hasData_ ? bands_.air : 0.08f,
    };
    const char* labels[] = { "SUB", "LOW", "MID", "HIGH", "AIR" };

    auto plot = inner.removeFromTop(inner.getHeight() - 22);
    const int n = 5;
    const float gap = 10.0f;
    const float barW = ((float) plot.getWidth() - gap * (float) (n - 1)) / (float) n;
    const float maxH = (float) plot.getHeight();

    juce::Path silhouette;
    for (int i = 0; i < n; ++i)
    {
        const float x = (float) plot.getX() + (barW + gap) * (float) i;
        const float h = maxH * juce::jlimit(0.04f, 1.0f, values[i] * 2.1f);
        const float y = (float) plot.getBottom() - h;
        g.setColour(hasData_ ? colors::foreground().withAlpha(0.22f) : colors::border());
        g.fillRect(x, y, barW, h);
        g.setColour(hasData_ ? colors::foreground().withAlpha(0.70f) : colors::mutedForeground());
        g.drawRect(x, y, barW, h, 1.0f);

        const float cx = x + barW * 0.5f;
        if (i == 0)
            silhouette.startNewSubPath(cx, y);
        else
            silhouette.lineTo(cx, y);
    }

    if (hasData_)
    {
        g.setColour(colors::foreground().withAlpha(0.85f));
        g.strokePath(silhouette, juce::PathStrokeType(1.2f));
    }

    auto legend = inner;
    const float cell = (float) legend.getWidth() / (float) n;
    g.setFont(type::label(9.0f));
    g.setColour(colors::mutedForeground());
    for (int i = 0; i < n; ++i)
    {
        auto cellBounds = juce::Rectangle<float>(
                              (float) legend.getX() + cell * (float) i,
                              (float) legend.getY(),
                              cell,
                              (float) legend.getHeight())
                              .toNearestInt();
        g.drawFittedText(labels[i], cellBounds, juce::Justification::centred, 1);
    }
}

} // namespace sonora
