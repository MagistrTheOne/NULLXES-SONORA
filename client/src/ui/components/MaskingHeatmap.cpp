#include "ui/components/MaskingHeatmap.h"

#include "ui/theme/Theme.h"

namespace sonora
{

MaskingHeatmap::MaskingHeatmap(AppState& state) : state_(state) {}

void MaskingHeatmap::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(16), "MASKING");
    bounds.removeFromTop(6);
    const auto* dna = state_.dna();
    if (dna == nullptr || dna->maskingRoles.size() != 4 || dna->maskingMatrix.size() != 4)
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Waiting");
        return;
    }

    const int cellW = bounds.getWidth() / 5;
    const int cellH = juce::jmax(16, bounds.getHeight() / 5);
    auto cell = [&](int x, int y) {
        return juce::Rectangle<int>(bounds.getX() + x * cellW, bounds.getY() + y * cellH, cellW - 3, cellH);
    };
    for (int i = 0; i < 4; ++i)
        Theme::drawMuted(g, cell(i + 1, 0), juce::String(dna->maskingRoles[(size_t) i]).substring(0, 4).toUpperCase());
    for (int y = 0; y < 4; ++y)
    {
        Theme::drawMuted(g, cell(0, y + 1), juce::String(dna->maskingRoles[(size_t) y]).substring(0, 4).toUpperCase());
        for (int x = 0; x < 4; ++x)
        {
            if (x == y)
            {
                Theme::drawMuted(g, cell(x + 1, y + 1), "-");
                continue;
            }
            const float value = x < (int) dna->maskingMatrix[(size_t) y].size()
                ? dna->maskingMatrix[(size_t) y][(size_t) x]
                : 0.0f;
            auto box = cell(x + 1, y + 1).reduced(1, 2);
            g.setColour((value >= 0.45f ? colors::destructive() : colors::foreground()).withAlpha(0.12f + value * 0.45f));
            g.fillRect(box);
            if (state_.uiMode() == UiMode::Advanced)
            {
                g.setColour(colors::foreground());
                g.setFont(type::label(9.0f));
                g.drawText(juce::String(juce::roundToInt(value * 100.0f)), box, juce::Justification::centred, true);
            }
        }
    }
}

} // namespace sonora
