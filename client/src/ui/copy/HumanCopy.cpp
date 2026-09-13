#include "ui/copy/HumanCopy.h"

#include "models/Insight.h"
#include "ui/theme/Theme.h"

namespace sonora::copy
{

juce::Colour toneColour(Tone tone)
{
    switch (tone)
    {
        case Tone::Attention:
            return colors::destructive();
        case Tone::Good:
            return colors::foreground();
        case Tone::Neutral:
        default:
            return colors::mutedForeground();
    }
}

juce::String riskStatus(const juce::String& risk)
{
    if (risk == "high")
        return "Needs attention";
    if (risk == "low")
        return "Good";
    return "Neutral";
}

Tone riskTone(const juce::String& risk)
{
    if (risk == "high")
        return Tone::Attention;
    if (risk == "low")
        return Tone::Good;
    return Tone::Neutral;
}

juce::String issuePhrase(const models::Issue& issue)
{
    if (issue.type == "muddy_low_end")
        return "Bass is too strong";
    if (issue.type == "frequency_conflict")
        return "Vocals need space";
    if (issue.type == "narrow_stereo")
        return "Stereo is narrow";
    if (issue.type == "clipping")
        return "Clipping risk";
    if (issue.type == "low_dynamic_range")
        return "The mix is over-compressed";
    if (!issue.detail.empty())
        return juce::String(issue.detail);
    return juce::String(issue.type);
}

juce::String formatTime(float seconds)
{
    if (seconds < 0.0f)
        seconds = 0.0f;
    const int total = juce::roundToInt(seconds);
    const int minutes = total / 60;
    const int rest = total % 60;
    return juce::String(minutes) + ":" + juce::String(rest).paddedLeft('0', 2);
}

juce::String sectionLabel(const std::string& name)
{
    const auto text = juce::String(name).toLowerCase();
    if (text.contains("intro"))
        return "INTRO";
    if (text.contains("build"))
        return "BUILD";
    if (text.contains("drop"))
        return "DROP";
    if (text.contains("break"))
        return "BREAK";
    if (text.contains("outro"))
        return "OUTRO";
    if (text.contains("verse"))
        return "VERSE";
    if (text.contains("chorus"))
        return "CHORUS";
    if (text.contains("bridge"))
        return "BRIDGE";
    if (name.empty())
        return "BODY";
    return juce::String(name).toUpperCase();
}

std::vector<IdentityAxis> sonicIdentity(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    const float low = analysis.bands.sub + analysis.bands.low;
    const float bright = analysis.bands.high + analysis.bands.air;
    const float mid = analysis.bands.mid;
    float dark = juce::jlimit(0.0f, 1.0f, (1.0f - bright * 1.85f) * 0.55f + low * 0.7f);
    float bass = juce::jlimit(0.0f, 1.0f, low * 1.55f);
    float wide = juce::jlimit(0.0f, 1.0f, analysis.stereoWidth * 1.45f);
    float vocal = juce::jlimit(0.0f, 1.0f, mid * 1.65f);
    if (analysis.hasDna)
    {
        bass = juce::jlimit(0.0f, 1.0f, 0.45f * bass + 0.55f * analysis.dna.lowEnd.value);
        if (analysis.dna.brightness.value > 0.0f)
            dark = juce::jlimit(0.0f, 1.0f, 0.5f * dark + 0.5f * (1.0f - analysis.dna.brightness.value));
        wide = juce::jlimit(0.0f, 1.0f, 0.4f * wide + 0.6f * analysis.dna.stereo.value);
    }
    for (const auto& issue : issues)
    {
        if (issue.type == "frequency_conflict")
            vocal = juce::jlimit(0.0f, 1.0f, vocal * (1.0f - issue.severity * 0.45f));
        if (issue.type == "narrow_stereo")
            wide = juce::jlimit(0.0f, 1.0f, wide * (1.0f - issue.severity * 0.5f));
        if (issue.type == "muddy_low_end")
            bass = juce::jlimit(0.0f, 1.0f, juce::jmax(bass, 0.55f + issue.severity * 0.4f));
    }
    return {
        { "Dark", dark },
        { "Heavy Bass", bass },
        { "Wide Space", wide },
        { "Vocal Focus", vocal },
    };
}

std::vector<models::AssistOption> labOptions()
{
    return {
        { "strengthen_drop", "Make drop stronger" },
        { "create_bass", "Create bass variation" },
        { "fix_vocal_space", "Open vocal space" },
        { "build_arrangement", "Build arrangement" },
    };
}

juce::String styleLine(const models::TrackDna* dna, const std::vector<juce::String>& profile)
{
    juce::ignoreUnused(dna, profile);
    return "Unresolved";
}

juce::String styleFromIdentity(const std::vector<IdentityAxis>& axes)
{
    std::vector<juce::String> tags;
    for (const auto& axis : axes)
        if (axis.value >= 0.52f)
            tags.push_back(axis.name);
    if (tags.empty() && !axes.empty())
    {
        auto best = axes.front();
        for (const auto& axis : axes)
            if (axis.value > best.value)
                best = axis;
        tags.push_back(best.name);
    }
    if (tags.size() > 2)
        tags.resize(2);
    if (tags.empty())
        return "Unresolved";
    if (tags.size() == 1)
        return tags.front();
    return tags[0] + "  /  " + tags[1];
}

std::vector<juce::String> moodTags(const models::AudioAnalysis& analysis)
{
    std::vector<juce::String> tags;
    const auto bright = analysis.bands.high + analysis.bands.air;
    if (bright < 0.18f || analysis.bands.sub + analysis.bands.low >= 0.48f)
        tags.emplace_back("Dark");
    float energy = analysis.hasDna ? analysis.dna.energyPeak : 0.0f;
    if (energy >= 0.62f || analysis.dynamicRangeDb < 8.0f)
        tags.emplace_back("Energetic");
    if (tags.empty())
        tags.emplace_back("Steady");
    return tags;
}

int healthScore(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    float penalty = 0.0f;
    for (const auto& issue : issues)
    {
        float weight = 12.0f;
        if (issue.type == "clipping")
            weight = 22.0f;
        else if (issue.type == "muddy_low_end")
            weight = 18.0f;
        else if (issue.type == "frequency_conflict")
            weight = 14.0f;
        else if (issue.type == "narrow_stereo")
            weight = 12.0f;
        else if (issue.type == "low_dynamic_range")
            weight = 10.0f;
        penalty += issue.severity * weight;
    }
    if (analysis.hasDna && analysis.dna.lowEnd.risk == "high")
        penalty += 8.0f;
    return juce::jlimit(0, 100, juce::roundToInt(100.0f - penalty));
}

juce::String healthVerdict(int score)
{
    if (score >= 80)
        return "Good";
    if (score >= 55)
        return "Some issues to improve";
    return "Needs attention";
}

namespace
{
const models::Issue* findIssue(const std::vector<models::Issue>& issues, const char* type)
{
    for (const auto& issue : issues)
        if (issue.type == type)
            return &issue;
    return nullptr;
}

MixRow row(const juce::String& name, float health, const juce::String& status, Tone tone)
{
    MixRow item;
    item.name = name;
    item.health = juce::jlimit(0.0f, 1.0f, health);
    item.status = status;
    item.tone = tone;
    return item;
}
} // namespace

std::vector<MixRow> mixRows(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    std::vector<MixRow> rows;
    const auto* muddy = findIssue(issues, "muddy_low_end");
    const auto* conflict = findIssue(issues, "frequency_conflict");
    const auto* narrow = findIssue(issues, "narrow_stereo");
    const auto* clip = findIssue(issues, "clipping");
    const auto* dyn = findIssue(issues, "low_dynamic_range");

    if (muddy != nullptr || (analysis.hasDna && analysis.dna.lowEnd.risk == "high"))
        rows.push_back(row("Bass", 1.0f - (muddy != nullptr ? muddy->severity : 0.72f), "Too strong", Tone::Attention));
    else if (analysis.hasDna && analysis.dna.lowEnd.risk == "medium")
        rows.push_back(row("Bass", 0.62f, "Full", Tone::Neutral));
    else
        rows.push_back(row("Bass", 0.86f, "Good", Tone::Good));

    if (conflict != nullptr)
        rows.push_back(row("Vocals", 1.0f - conflict->severity, "Needs space", Tone::Attention));
    else
        rows.push_back(row("Vocals", 0.78f, "Has room", Tone::Good));

    if (narrow != nullptr || analysis.channels == 1)
        rows.push_back(row("Stereo", juce::jlimit(0.15f, 0.55f, analysis.stereoWidth * 4.0f), "Narrow", Tone::Attention));
    else if (analysis.stereoWidth > 0.62f)
        rows.push_back(row("Stereo", 0.58f, "Wide", Tone::Neutral));
    else
        rows.push_back(row("Stereo", 0.80f, "Good", Tone::Good));

    if (dyn != nullptr)
        rows.push_back(row("Dynamics", 1.0f - dyn->severity, "Flat", Tone::Attention));
    else
        rows.push_back(row("Dynamics", juce::jlimit(0.4f, 1.0f, analysis.dynamicRangeDb / 16.0f), "Good", Tone::Good));

    if (clip != nullptr)
        rows.push_back(row("Clipping", 1.0f - clip->severity, "Risk", Tone::Attention));
    else
        rows.push_back(row("Clipping", 0.92f, "Clean", Tone::Good));

    return rows;
}

Finding assistFinding(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    Finding finding;
    if (const auto* muddy = findIssue(issues, "muddy_low_end"))
    {
        finding.headline = "Bass is crowding the mix.";
        finding.detail = "Low end dominates below 120Hz and may mask the kick. Open space around 100-150Hz.";
        juce::ignoreUnused(muddy);
        return finding;
    }
    if (const auto* conflict = findIssue(issues, "frequency_conflict"))
    {
        finding.headline = "Vocals need space.";
        finding.detail = "Bass and mid information are fighting. Carve a pocket instead of boosting everything.";
        juce::ignoreUnused(conflict);
        return finding;
    }
    if (const auto* clip = findIssue(issues, "clipping"))
    {
        finding.headline = "Peaks are hitting the ceiling.";
        finding.detail = "Reduce input gain before the mix can open up.";
        juce::ignoreUnused(clip);
        return finding;
    }
    if (const auto* narrow = findIssue(issues, "narrow_stereo"))
    {
        finding.headline = "The image is too narrow.";
        finding.detail = "Width above 400Hz will give the track air without touching the center.";
        juce::ignoreUnused(narrow);
        return finding;
    }

    if (analysis.hasDna)
    {
        for (const auto& section : analysis.dna.sections)
        {
            if (section.name == "drop" && section.energy >= 0.7f)
            {
                finding.headline = "The drop works.";
                finding.detail = "Energy lands at " + formatTime(section.start) + ". Keep the body, fix the mask around it.";
                return finding;
            }
        }
        for (const auto& target : analysis.dna.translation)
        {
            if (!target.issue.empty())
            {
                finding.headline = juce::String(target.issue);
                finding.detail = juce::String(target.reason);
                if (!target.action.empty())
                    finding.detail += " " + juce::String(target.action);
                return finding;
            }
        }
    }

    finding.headline = "The track has a solid foundation.";
    finding.detail = "No urgent collision. Create the next object when you want to move.";
    return finding;
}

Delta mixDelta(const models::AudioAnalysis& previous, const models::AudioAnalysis& current)
{
    Delta delta;
    delta.valid = true;
    delta.loudness = current.loudnessLufsApprox - previous.loudnessLufsApprox;
    const float prevLow = previous.hasDna ? previous.dna.lowEnd.control : 0.0f;
    const float nextLow = current.hasDna ? current.dna.lowEnd.control : 0.0f;
    delta.lowEnd = nextLow - prevLow;
    delta.stereo = current.stereoWidth - previous.stereoWidth;
    const float prevEnergy = previous.hasDna ? previous.dna.energyMean : 0.0f;
    const float nextEnergy = current.hasDna ? current.dna.energyMean : 0.0f;
    delta.energy = nextEnergy - prevEnergy;
    return delta;
}

juce::String signedPercent(float value)
{
    const int pct = juce::roundToInt(value * 100.0f);
    if (pct > 0)
        return "+" + juce::String(pct) + "%";
    return juce::String(pct) + "%";
}

std::vector<juce::String> foundLines(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    std::vector<juce::String> lines;
    const auto* muddy = findIssue(issues, "muddy_low_end");
    const auto* vocal = findIssue(issues, "frequency_conflict");
    const auto* clip = findIssue(issues, "clipping");
    const auto* narrow = findIssue(issues, "narrow_stereo");
    const auto* dyn = findIssue(issues, "low_dynamic_range");

    if (muddy != nullptr)
        lines.emplace_back("Bass dominates");
    if (vocal != nullptr)
        lines.emplace_back("Vocal needs space");

    bool hasDrop = false;
    float dropEnergy = 0.0f;
    float bodyEnergy = analysis.hasDna ? analysis.dna.energyMean : 0.0f;
    if (analysis.hasDna)
    {
        for (const auto& section : analysis.dna.sections)
        {
            if (section.name == "drop")
            {
                hasDrop = true;
                dropEnergy = juce::jmax(dropEnergy, section.energy);
            }
        }
    }
    if (analysis.hasDna && (!hasDrop || dropEnergy < bodyEnergy + 0.08f) && lines.size() < 3)
        lines.emplace_back("Drop lacks contrast");
    if (narrow != nullptr)
        lines.emplace_back("Stereo is closed");
    if (clip != nullptr)
        lines.emplace_back("Peaks are hot");
    if (dyn != nullptr && lines.size() < 4)
        lines.emplace_back("Dynamics are flat");
    if (lines.empty())
        lines.emplace_back("Foundation is solid");
    if (lines.size() > 3)
        lines.resize(3);
    return lines;
}

juce::String structureLine(const models::TrackDna* dna)
{
    if (dna == nullptr || dna->sections.empty())
        return "BODY";
    juce::StringArray names;
    for (const auto& section : dna->sections)
    {
        const auto label = sectionLabel(section.name);
        if (!names.contains(label))
            names.add(label);
    }
    if (names.isEmpty())
        return "BODY";
    return names.joinIntoString("   ");
}

juce::String mixLine(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    juce::StringArray bits;
    if (findIssue(issues, "muddy_low_end") != nullptr)
        bits.add("Low end heavy");
    if (findIssue(issues, "frequency_conflict") != nullptr)
        bits.add("Vocal space limited");
    if (findIssue(issues, "clipping") != nullptr)
        bits.add("Clipping risk");
    if (findIssue(issues, "narrow_stereo") != nullptr)
        bits.add("Narrow image");
    if (bits.isEmpty())
    {
        if (analysis.bands.sub + analysis.bands.low >= 0.48f)
            bits.add("Low end heavy");
        else
            bits.add(healthVerdict(healthScore(analysis, issues)));
    }
    return bits.joinIntoString("  ·  ");
}

} // namespace sonora::copy
