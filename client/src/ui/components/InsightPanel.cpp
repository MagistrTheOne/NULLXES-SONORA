#include "ui/components/InsightPanel.h"

#include "ui/theme/Theme.h"

namespace sonora
{
namespace
{
void drawField(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::String& key, const juce::String& value)
{
    theme::drawSectionLabel(g, bounds.removeFromTop(12), key);
    bounds.removeFromTop(4);
    g.setColour(colors::foreground());
    g.setFont(type::body(13.0f));
    g.drawText(value, bounds.removeFromTop(18), juce::Justification::centredLeft, true);
    bounds.removeFromTop(10);
}

juce::String objectLine(const models::SonoraObject& object)
{
    juce::String line(object.type);
    if (object.frequency > 0.0f)
        line << "  " << juce::roundToInt(object.frequency) << "Hz";
    if (object.gain != 0.0f)
        line << "  " << juce::String(object.gain, 1) << "dB";
    return line;
}
} // namespace

InsightPanel::InsightPanel(AppState& state)
    : state_(state)
{
    generate_.setLabel("GENERATE ENGINEERING REPORT");
    generate_.onClick = [this] { state_.requestEngineeringReport(); };
    createEq_.setLabel("CREATE EQ PROFILE");
    createEq_.onClick = [this] { state_.createEqProfile(); };
    addAndMakeVisible(generate_);
    addAndMakeVisible(createEq_);
}

void InsightPanel::paint(juce::Graphics& g)
{
    theme::fillCard(g, getLocalBounds());

    auto bounds = getLocalBounds().reduced(16, 14);
    const auto node = state_.selectedNode();
    juce::String title = "INSPECTOR";
    switch (node)
    {
        case CanvasNode::Input: title = "INPUT"; break;
        case CanvasNode::Dna: title = "TRACK DNA"; break;
        case CanvasNode::Structure: title = "STRUCTURE"; break;
        case CanvasNode::Mix: title = "MIX ENGINE"; break;
        case CanvasNode::Harmony: title = "HARMONY"; break;
        case CanvasNode::Export: title = "OBJECTS"; break;
    }
    theme::drawSectionLabel(g, bounds.removeFromTop(16), title);
    bounds.removeFromTop(10);
    bounds.removeFromBottom(76);

    if (state_.analysisState() != AnalysisState::Complete)
    {
        g.setColour(colors::mutedForeground());
        g.setFont(type::body(12.0f));
        g.drawMultiLineText(
            "Load a track. SONORA builds an engineering model, then objects.",
            bounds.getX(),
            bounds.getY() + 14,
            bounds.getWidth());
        return;
    }

    const auto* dna = state_.dna();

    if (node == CanvasNode::Input)
    {
        drawField(g, bounds, "TRACK", juce::String(state_.loadedFilename()));
        drawField(g, bounds, "FORMAT", state_.channelLabel() + "  /  " + state_.sampleRateLabel());
        drawField(g, bounds, "ENGINE", "DSP READY");
        return;
    }

    if (node == CanvasNode::Dna && dna != nullptr)
    {
        drawField(g, bounds, "TEMPO", juce::String(juce::roundToInt(dna->tempo)));
        drawField(g, bounds, "KEY", dna->keyName.empty() ? juce::String("---") : juce::String(dna->keyName));
        drawField(g, bounds, "CONFIDENCE", juce::String(juce::roundToInt(dna->keyConfidence * 100.0f)) + "%");
        juce::String genres;
        for (const auto& tag : dna->genreProfile)
        {
            if (genres.isNotEmpty())
                genres << "  ";
            genres << juce::String(tag);
        }
        drawField(g, bounds, "GENRE PROFILE", genres.isNotEmpty() ? genres : juce::String("unresolved"));
        return;
    }

    if (node == CanvasNode::Structure && dna != nullptr && !dna->sections.empty())
    {
        drawField(g, bounds, "SECTIONS", juce::String((int) dna->sections.size()));
        drawField(g, bounds, "FIRST", juce::String(dna->sections.front().name).toUpperCase()
            + "  " + juce::String(dna->sections.front().start, 1));
        drawField(g, bounds, "LAST", juce::String(dna->sections.back().name).toUpperCase()
            + "  " + juce::String(dna->sections.back().end, 1));
        drawField(g, bounds, "ENERGY PEAK", juce::String(dna->energyPeak, 2));
        return;
    }

    if (node == CanvasNode::Mix && dna != nullptr)
    {
        drawField(g, bounds, "LOW END RISK", juce::String(dna->lowEnd.risk).toUpperCase());
        drawField(g, bounds, "WHY", juce::String(dna->lowEnd.why));
        drawField(g, bounds, "CONTROL", juce::String(dna->lowEnd.control, 2));
        if (state_.eqProfile())
            drawField(g, bounds, "EQ OBJECT",
                      juce::String(juce::roundToInt(state_.eqProfile()->frequencyHz)) + "Hz  "
                          + juce::String(state_.eqProfile()->gainDb, 1) + "dB");
        else
            drawField(g, bounds, "EQ OBJECT", "not created");
        return;
    }

    if (node == CanvasNode::Harmony)
    {
        if (state_.harmony())
        {
            drawField(g, bounds, "KEY", juce::String(state_.harmony()->key));
            drawField(g, bounds, "BARS", juce::String(state_.harmony()->bars));
            juce::String line;
            for (const auto& chord : state_.harmony()->chords)
            {
                if (line.isNotEmpty())
                    line << "  ";
                line << juce::String(chord);
            }
            drawField(g, bounds, "MIDI CLIP", line);
        }
        else
        {
            drawField(g, bounds, "OBJECT", "waiting");
            drawField(g, bounds, "ACTION", "CREATE -> CHORD PROGRESSION");
        }
        return;
    }

    if (node == CanvasNode::Export && dna != nullptr)
    {
        drawField(g, bounds, "COUNT", juce::String((int) dna->objects.size()));
        int shown = 0;
        for (const auto& object : dna->objects)
        {
            if (shown >= 3)
                break;
            drawField(g, bounds, juce::String(object.type), objectLine(object));
            ++shown;
        }
        return;
    }

    juce::String issue = "No issues yet";
    juce::String confidence = "---";
    juce::String action = "Generate engineering report";
    juce::String target = "120Hz";

    if (!state_.insights().empty())
    {
        const auto& insight = state_.insights().front();
        issue = insight.reason.empty() ? insight.issue : insight.reason;
        if (issue.isEmpty())
            issue = insight.action;
        confidence = juce::String(juce::roundToInt(insight.confidence * 100.0f)) + "%";
        action = insight.action.empty() ? "Create EQ profile" : juce::String(insight.action);
        if (insight.frequencyHz.has_value())
            target = juce::String(juce::roundToInt(*insight.frequencyHz)) + "Hz";
    }
    else if (!state_.issues().empty())
    {
        const auto& first = state_.issues().front();
        issue = first.detail.empty() ? first.type : first.detail;
        confidence = juce::String(juce::roundToInt(first.severity * 100.0f)) + "%";
        action = "Create EQ profile";
    }

    drawField(g, bounds, "ISSUE", issue);
    drawField(g, bounds, "CONFIDENCE", confidence);
    drawField(g, bounds, "ACTION", action);
    drawField(g, bounds, "TARGET", target);
}

void InsightPanel::resized()
{
    auto bounds = getLocalBounds().reduced(16, 14);
    createEq_.setBounds(bounds.removeFromBottom(32));
    bounds.removeFromBottom(8);
    generate_.setBounds(bounds.removeFromBottom(32));
}

} // namespace sonora
