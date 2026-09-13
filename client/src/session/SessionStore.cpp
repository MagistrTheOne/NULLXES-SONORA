#include "session/SessionStore.h"

#include "app/Version.h"

namespace sonora::session
{
namespace
{
float num(const juce::var& value, float fallback = 0.0f)
{
    if (value.isVoid() || value.isUndefined())
        return fallback;
    return (float) (double) value;
}

int integer(const juce::var& value, int fallback = 0)
{
    if (value.isVoid() || value.isUndefined())
        return fallback;
    return (int) value;
}

juce::String text(const juce::var& value)
{
    return value.toString();
}

juce::var object()
{
    return juce::var(new juce::DynamicObject());
}

void set(juce::var& target, const juce::Identifier& key, const juce::var& value)
{
    if (auto* obj = target.getDynamicObject())
        obj->setProperty(key, value);
}

juce::var get(const juce::var& target, const juce::Identifier& key)
{
    if (auto* obj = target.getDynamicObject())
        return obj->getProperty(key);
    return {};
}

juce::var strings(const std::vector<std::string>& items)
{
    juce::Array<juce::var> rows;
    for (const auto& item : items)
        rows.add(juce::String(item));
    return rows;
}

std::vector<std::string> readStrings(const juce::var& value)
{
    std::vector<std::string> rows;
    if (auto* list = value.getArray())
        for (const auto& item : *list)
            rows.push_back(item.toString().toStdString());
    return rows;
}

juce::var axisVar(const models::MixAxis& axis)
{
    auto row = object();
    set(row, "sub", axis.sub);
    set(row, "low", axis.low);
    set(row, "control", axis.control);
    set(row, "value", axis.value);
    set(row, "risk", juce::String(axis.risk));
    set(row, "why", juce::String(axis.why));
    set(row, "method", juce::String(axis.method));
    return row;
}

models::MixAxis readAxis(const juce::var& value)
{
    models::MixAxis axis;
    axis.sub = num(get(value, "sub"));
    axis.low = num(get(value, "low"));
    axis.control = num(get(value, "control"));
    axis.value = num(get(value, "value"));
    axis.risk = text(get(value, "risk")).toStdString();
    axis.why = text(get(value, "why")).toStdString();
    axis.method = text(get(value, "method")).toStdString();
    return axis;
}

juce::var clipVar(const models::MidiClip& clip)
{
    auto row = object();
    set(row, "role", juce::String(clip.role));
    set(row, "key", juce::String(clip.key));
    set(row, "bars", clip.bars);
    set(row, "chords", strings(clip.chords));
    set(row, "notes", strings(clip.notes));
    set(row, "pattern", strings(clip.pattern));
    return row;
}

models::MidiClip readClip(const juce::var& value)
{
    models::MidiClip clip;
    clip.role = text(get(value, "role")).toStdString();
    clip.key = text(get(value, "key")).toStdString();
    clip.bars = integer(get(value, "bars"), 8);
    clip.chords = readStrings(get(value, "chords"));
    clip.notes = readStrings(get(value, "notes"));
    clip.pattern = readStrings(get(value, "pattern"));
    return clip;
}

juce::var analysisVar(const models::AudioAnalysis& analysis)
{
    auto row = object();
    set(row, "analyzer", juce::String(analysis.analyzerVersion));
    set(row, "bpm", analysis.bpm);
    set(row, "duration", analysis.durationSec);
    set(row, "sampleRate", analysis.sampleRate);
    set(row, "channels", analysis.channels);
    set(row, "loudness", analysis.loudnessLufsApprox);
    set(row, "dynamic", analysis.dynamicRangeDb);
    set(row, "stereo", analysis.stereoWidth);
    if (analysis.key.key.has_value())
        set(row, "key", juce::String(*analysis.key.key));
    set(row, "keyConfidence", analysis.key.confidence);
    set(row, "keyMethod", juce::String(analysis.key.method));
    auto bands = object();
    set(bands, "sub", analysis.bands.sub);
    set(bands, "low", analysis.bands.low);
    set(bands, "mid", analysis.bands.mid);
    set(bands, "high", analysis.bands.high);
    set(bands, "air", analysis.bands.air);
    set(row, "bands", bands);
    set(row, "hasDna", analysis.hasDna);
    if (!analysis.hasDna)
        return row;

    const auto& dna = analysis.dna;
    auto dnaVar = object();
    set(dnaVar, "tempo", dna.tempo);
    set(dnaVar, "keyName", juce::String(dna.keyName));
    set(dnaVar, "keyConfidence", dna.keyConfidence);
    set(dnaVar, "genre", strings(dna.genreProfile));
    set(dnaVar, "energyMean", dna.energyMean);
    set(dnaVar, "energyPeak", dna.energyPeak);
    set(dnaVar, "lowEnd", axisVar(dna.lowEnd));
    set(dnaVar, "brightness", axisVar(dna.brightness));
    set(dnaVar, "stereo", axisVar(dna.stereo));
    set(dnaVar, "dynamics", axisVar(dna.dynamics));
    juce::Array<juce::var> sections;
    for (const auto& section : dna.sections)
    {
        auto item = object();
        set(item, "name", juce::String(section.name));
        set(item, "start", section.start);
        set(item, "end", section.end);
        set(item, "energy", section.energy);
        set(item, "bassEnergy", section.bassEnergy);
        set(item, "transientDensity", section.transientDensity);
        set(item, "stereoWidth", section.stereoWidth);
        sections.add(item);
    }
    set(dnaVar, "sections", sections);
    juce::Array<juce::var> objects;
    for (const auto& objectItem : dna.objects)
    {
        auto item = object();
        set(item, "type", juce::String(objectItem.type));
        set(item, "input", juce::String(objectItem.input));
        set(item, "status", juce::String(objectItem.status));
        set(item, "frequency", objectItem.frequency);
        set(item, "gain", objectItem.gain);
        set(item, "q", objectItem.q);
        objects.add(item);
    }
    set(dnaVar, "objects", objects);
    set(row, "dna", dnaVar);
    return row;
}

models::AudioAnalysis readAnalysis(const juce::var& value)
{
    models::AudioAnalysis analysis;
    analysis.analyzerVersion = text(get(value, "analyzer")).toStdString();
    analysis.bpm = num(get(value, "bpm"));
    analysis.durationSec = num(get(value, "duration"));
    analysis.sampleRate = integer(get(value, "sampleRate"));
    analysis.channels = integer(get(value, "channels"));
    analysis.loudnessLufsApprox = num(get(value, "loudness"));
    analysis.dynamicRangeDb = num(get(value, "dynamic"));
    analysis.stereoWidth = num(get(value, "stereo"));
    const auto key = text(get(value, "key"));
    if (key.isNotEmpty())
        analysis.key.key = key.toStdString();
    analysis.key.confidence = num(get(value, "keyConfidence"));
    analysis.key.method = text(get(value, "keyMethod")).toStdString();
    const auto bands = get(value, "bands");
    analysis.bands.sub = num(get(bands, "sub"));
    analysis.bands.low = num(get(bands, "low"));
    analysis.bands.mid = num(get(bands, "mid"));
    analysis.bands.high = num(get(bands, "high"));
    analysis.bands.air = num(get(bands, "air"));
    analysis.hasDna = (bool) get(value, "hasDna");
    if (!analysis.hasDna)
        return analysis;

    const auto dnaVar = get(value, "dna");
    auto& dna = analysis.dna;
    dna.tempo = num(get(dnaVar, "tempo"));
    dna.keyName = text(get(dnaVar, "keyName")).toStdString();
    dna.keyConfidence = num(get(dnaVar, "keyConfidence"));
    dna.genreProfile = readStrings(get(dnaVar, "genre"));
    dna.energyMean = num(get(dnaVar, "energyMean"));
    dna.energyPeak = num(get(dnaVar, "energyPeak"));
    dna.lowEnd = readAxis(get(dnaVar, "lowEnd"));
    dna.brightness = readAxis(get(dnaVar, "brightness"));
    dna.stereo = readAxis(get(dnaVar, "stereo"));
    dna.dynamics = readAxis(get(dnaVar, "dynamics"));
    if (auto* sections = get(dnaVar, "sections").getArray())
    {
        for (const auto& item : *sections)
        {
            models::StructureSection section;
            section.name = text(get(item, "name")).toStdString();
            section.start = num(get(item, "start"));
            section.end = num(get(item, "end"));
            section.energy = num(get(item, "energy"));
            section.bassEnergy = num(get(item, "bassEnergy"));
            section.transientDensity = num(get(item, "transientDensity"));
            section.stereoWidth = num(get(item, "stereoWidth"));
            dna.sections.push_back(section);
        }
    }
    if (auto* objects = get(dnaVar, "objects").getArray())
    {
        for (const auto& item : *objects)
        {
            models::SonoraObject objectItem;
            objectItem.type = text(get(item, "type")).toStdString();
            objectItem.input = text(get(item, "input")).toStdString();
            objectItem.status = text(get(item, "status")).toStdString();
            objectItem.frequency = num(get(item, "frequency"));
            objectItem.gain = num(get(item, "gain"));
            objectItem.q = num(get(item, "q"));
            dna.objects.push_back(objectItem);
        }
    }
    return analysis;
}
} // namespace

juce::File defaultFile()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("NULLXES")
        .getChildFile("SONORA")
        .getChildFile("session.sonora");
}

juce::String encode(const Blob& blob)
{
    auto root = object();
    set(root, "version", juce::String(kVersion));
    set(root, "name", blob.name);
    set(root, "tab", blob.tab);
    set(root, "met", blob.met);
    set(root, "understood", blob.understood);
    if (blob.hasAnalysis)
        set(root, "analysis", analysisVar(blob.analysis));
    juce::Array<juce::var> issues;
    for (const auto& issue : blob.issues)
    {
        auto row = object();
        set(row, "type", juce::String(issue.type));
        set(row, "severity", issue.severity);
        set(row, "area", juce::String(issue.area));
        set(row, "detail", juce::String(issue.detail));
        issues.add(row);
    }
    set(root, "issues", issues);
    if (blob.harmony)
    {
        auto row = object();
        set(row, "key", juce::String(blob.harmony->key));
        set(row, "bars", blob.harmony->bars);
        set(row, "chords", strings(blob.harmony->chords));
        set(root, "harmony", row);
    }
    if (blob.bass)
        set(root, "bass", clipVar(*blob.bass));
    if (blob.pad)
        set(root, "pad", clipVar(*blob.pad));
    if (blob.drop)
    {
        auto row = object();
        set(row, "sectionName", juce::String(blob.drop->sectionName));
        set(row, "start", blob.drop->start);
        set(row, "end", blob.drop->end);
        set(row, "actions", strings(blob.drop->actions));
        set(row, "frequency", blob.drop->frequency);
        set(row, "gain", blob.drop->gain);
        set(row, "q", blob.drop->q);
        set(row, "energyTarget", blob.drop->energyTarget);
        set(root, "drop", row);
    }
    if (blob.eq)
    {
        auto row = object();
        set(row, "operation", juce::String(blob.eq->operation));
        set(row, "frequencyHz", blob.eq->frequencyHz);
        set(row, "gainDb", blob.eq->gainDb);
        set(row, "target", juce::String(blob.eq->target));
        set(root, "eq", row);
    }
    juce::Array<juce::var> chat;
    for (const auto& line : blob.soni)
    {
        auto row = object();
        set(row, "fromSoni", line.fromSoni);
        set(row, "text", line.text);
        chat.add(row);
    }
    set(root, "soni", chat);
    return juce::JSON::toString(root, false);
}

bool decode(const juce::String& json, Blob& blob)
{
    const auto parsed = juce::JSON::parse(json);
    if (!parsed.isObject())
        return false;
    blob = {};
    blob.version = text(get(parsed, "version"));
    blob.name = text(get(parsed, "name"));
    blob.tab = text(get(parsed, "tab"));
    if (blob.tab.isEmpty())
        blob.tab = "listen";
    blob.met = (bool) get(parsed, "met");
    blob.understood = (bool) get(parsed, "understood");
    const auto analysis = get(parsed, "analysis");
    if (analysis.isObject())
    {
        blob.hasAnalysis = true;
        blob.understood = true;
        blob.analysis = readAnalysis(analysis);
    }
    if (auto* issues = get(parsed, "issues").getArray())
    {
        for (const auto& item : *issues)
        {
            models::Issue issue;
            issue.type = text(get(item, "type")).toStdString();
            issue.severity = num(get(item, "severity"));
            issue.area = text(get(item, "area")).toStdString();
            issue.detail = text(get(item, "detail")).toStdString();
            blob.issues.push_back(issue);
        }
    }
    const auto harmony = get(parsed, "harmony");
    if (harmony.isObject())
    {
        models::Harmony row;
        row.key = text(get(harmony, "key")).toStdString();
        row.bars = integer(get(harmony, "bars"), 8);
        row.chords = readStrings(get(harmony, "chords"));
        blob.harmony = row;
    }
    const auto bass = get(parsed, "bass");
    if (bass.isObject())
        blob.bass = readClip(bass);
    const auto pad = get(parsed, "pad");
    if (pad.isObject())
        blob.pad = readClip(pad);
    const auto drop = get(parsed, "drop");
    if (drop.isObject())
    {
        models::DropPlan row;
        row.sectionName = text(get(drop, "sectionName")).toStdString();
        row.start = num(get(drop, "start"));
        row.end = num(get(drop, "end"));
        row.actions = readStrings(get(drop, "actions"));
        row.frequency = num(get(drop, "frequency"), 3000.0f);
        row.gain = num(get(drop, "gain"), 2.5f);
        row.q = num(get(drop, "q"), 1.1f);
        row.energyTarget = num(get(drop, "energyTarget"), 0.85f);
        blob.drop = row;
    }
    const auto eq = get(parsed, "eq");
    if (eq.isObject())
    {
        models::EqProfile row;
        row.operation = text(get(eq, "operation")).toStdString();
        row.frequencyHz = num(get(eq, "frequencyHz"), 120.0f);
        row.gainDb = num(get(eq, "gainDb"), -3.0f);
        row.target = text(get(eq, "target")).toStdString();
        blob.eq = row;
    }
    if (auto* chat = get(parsed, "soni").getArray())
    {
        for (const auto& item : *chat)
            blob.soni.push_back({ (bool) get(item, "fromSoni"), text(get(item, "text")) });
    }
    return true;
}

bool save(const juce::String& json)
{
    const auto file = defaultFile();
    file.getParentDirectory().createDirectory();
    return file.replaceWithText(json);
}

juce::String load()
{
    const auto file = defaultFile();
    if (!file.existsAsFile())
        return {};
    return file.loadFileAsString();
}

} // namespace sonora::session
