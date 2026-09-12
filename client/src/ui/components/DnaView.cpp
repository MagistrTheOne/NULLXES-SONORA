#include "ui/components/DnaView.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
juce::String upper(const std::string& value)
{
    return juce::String(value).toUpperCase();
}

void drawAxisRow(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& label, float value)
{
    auto row = bounds.removeFromTop(18);
    Theme::drawMuted(g, row.removeFromLeft(88), label);
    Theme::drawBlocks(g, row.removeFromLeft(juce::jmax(80, row.getWidth() - 48)).reduced(0, 4), value);
    g.setColour(colors::foreground());
    g.setFont(type::label(10.0f));
    g.drawText(juce::String(value, 2), row, juce::Justification::centredRight, true);
    bounds.removeFromTop(6);
}
} // namespace

DnaView::DnaView(AppState& state) : state_(state)
{
    setOpaque(false);
}

void DnaView::paint(juce::Graphics& g)
{
    const auto* dna = state_.dna();
    if (dna == nullptr)
        return;

    auto bounds = getLocalBounds();
    auto top = bounds.removeFromTop(juce::jmax(220, bounds.getHeight() * 55 / 100));
    auto mixBox = top.removeFromLeft(juce::jmax(240, top.getWidth() * 52 / 100));
    top.removeFromLeft(12);
    auto transBox = top;

    theme::fillCard(g, mixBox);
    auto mix = mixBox.reduced(16, 14);
    theme::drawSectionLabel(g, mix.removeFromTop(14), "LOW END CHARACTER");
    mix.removeFromTop(10);
    drawAxisRow(g, mix, "SUB", dna->lowEnd.sub);
    drawAxisRow(g, mix, "LOW", dna->lowEnd.low);
    drawAxisRow(g, mix, "CONTROL", dna->lowEnd.control);
    mix.removeFromTop(4);
    Theme::drawMuted(g, mix.removeFromTop(12), "RISK");
    mix.removeFromTop(4);
    g.setColour(Theme::riskColour(dna->lowEnd.risk));
    g.setFont(type::label(12.0f));
    g.drawText(upper(dna->lowEnd.risk), mix.removeFromTop(16), juce::Justification::centredLeft, true);
    mix.removeFromTop(10);
    Theme::drawMuted(g, mix.removeFromTop(12), "WHY");
    mix.removeFromTop(4);
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawText(juce::String(dna->lowEnd.why), mix.removeFromTop(18), juce::Justification::centredLeft, true);
    mix.removeFromTop(10);
    Theme::drawMuted(g, mix.removeFromTop(12), "METHOD");
    mix.removeFromTop(4);
    Theme::drawBody(g, mix.removeFromTop(16), juce::String(dna->lowEnd.method));

    theme::fillCard(g, transBox);
    auto trans = transBox.reduced(16, 14);
    theme::drawSectionLabel(g, trans.removeFromTop(14), "TRANSLATION");
    trans.removeFromTop(10);
    const models::TranslationTarget* worst = nullptr;
    for (const auto& target : dna->translation)
    {
        drawAxisRow(g, trans, upper(target.name), target.score);
        if (!target.issue.empty() && (worst == nullptr || target.score < worst->score))
            worst = &target;
    }
    trans.removeFromTop(6);
    if (worst != nullptr)
    {
        Theme::drawMuted(g, trans.removeFromTop(12), "ISSUE");
        trans.removeFromTop(4);
        Theme::drawBody(g, trans.removeFromTop(16), juce::String(worst->issue));
        trans.removeFromTop(8);
        Theme::drawMuted(g, trans.removeFromTop(12), "REASON");
        trans.removeFromTop(4);
        Theme::drawBody(g, trans.removeFromTop(16), juce::String(worst->reason));
        trans.removeFromTop(8);
        Theme::drawMuted(g, trans.removeFromTop(12), "ACTION");
        trans.removeFromTop(4);
        Theme::drawBody(g, trans.removeFromTop(16), juce::String(worst->action));
    }
    else
    {
        Theme::drawMuted(g, trans.removeFromTop(16), "No translation failure");
    }

    bounds.removeFromTop(12);
    theme::fillCard(g, bounds);
    auto mask = bounds.reduced(16, 14);
    theme::drawSectionLabel(g, mask.removeFromTop(14), "MASKING MATRIX");
    mask.removeFromTop(6);
    Theme::drawMuted(g, mask.removeFromTop(14), "FREQUENCY ROLE OVERLAP  /  NOT STEMS");
    mask.removeFromTop(8);
    if (dna->maskingRoles.size() != 4 || dna->maskingMatrix.size() != 4)
        return;

    const int cellW = mask.getWidth() / 5;
    const int cellH = juce::jmax(18, mask.getHeight() / 5);
    auto cell = [&](int x, int y) {
        return juce::Rectangle<int>(mask.getX() + x * cellW, mask.getY() + y * cellH, cellW - 4, cellH);
    };
    for (int i = 0; i < 4; ++i)
        Theme::drawMuted(g, cell(i + 1, 0), upper(dna->maskingRoles[(size_t) i]));
    for (int y = 0; y < 4; ++y)
    {
        Theme::drawMuted(g, cell(0, y + 1), upper(dna->maskingRoles[(size_t) y]));
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
            g.setColour(value >= 0.45f ? colors::warning() : colors::foreground());
            g.setFont(type::label(10.0f));
            g.drawText(
                juce::String(juce::roundToInt(value * 100.0f)) + "%",
                cell(x + 1, y + 1),
                juce::Justification::centredLeft,
                true);
        }
    }
}

} // namespace sonora
