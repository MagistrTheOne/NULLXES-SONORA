#include "ui/components/TranslationStrip.h"

#include "ui/theme/Theme.h"

namespace sonora
{

TranslationStrip::TranslationStrip(AppState& state) : state_(state) {}

void TranslationStrip::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());
    auto bounds = getLocalBounds().reduced(16, 12);
    Theme::drawMuted(g, bounds.removeFromTop(16), "TRANSLATION");
    bounds.removeFromTop(8);
    const auto* dna = state_.dna();
    if (dna == nullptr || dna->translation.empty())
    {
        Theme::drawMuted(g, bounds.removeFromTop(18), "Waiting");
        return;
    }

    for (const auto& target : dna->translation)
    {
        auto row = bounds.removeFromTop(22);
        Theme::drawMuted(g, row.removeFromLeft(88), juce::String(target.name).toUpperCase());
        Theme::drawBlocks(g, row.removeFromLeft(juce::jmax(70, row.getWidth() - 40)).reduced(0, 5), target.score);
        Theme::drawMuted(g, row, juce::String(target.score, 2));
        bounds.removeFromTop(4);
    }

    if (state_.uiMode() == UiMode::Advanced)
    {
        bounds.removeFromTop(6);
        Theme::drawMuted(g, bounds.removeFromTop(14), "METHOD");
        Theme::drawBody(g, bounds.removeFromTop(16), "band-weight playback models");
    }
}

} // namespace sonora
