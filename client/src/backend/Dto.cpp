#include "backend/Dto.h"

namespace sonora::dto
{
namespace
{
juce::DynamicObject* obj(const juce::var& v)
{
    return v.getDynamicObject();
}

float num(const juce::var& v, const char* key, float fallback = 0.0f)
{
    auto* o = obj(v);
    if (o == nullptr)
        return fallback;
    return (float) o->getProperty(key);
}

juce::String str(const juce::var& v, const char* key)
{
    auto* o = obj(v);
    if (o == nullptr)
        return {};
    return o->getProperty(key).toString();
}

const juce::Array<juce::var>* arr(const juce::var& v, const char* key)
{
    return v.getProperty(key, {}).getArray();
}
} // namespace

bool parseHealthOk(const juce::var& json)
{
    return str(json, "database") == "ok" || str(json, "status") == "ok";
}

juce::String parseAudioId(const juce::var& json)
{
    auto id = str(json, "audio_id");
    if (id.isNotEmpty())
        return id;
    if (auto* analysis = obj(json.getProperty("analysis", {})))
    {
        const auto nested = analysis->getProperty("audio_id").toString();
        if (nested.isNotEmpty())
            return nested;
    }
    if (auto* asset = obj(json.getProperty("asset", {})))
        return asset->getProperty("id").toString();
    return str(json, "id");
}

juce::String parseAnalysisId(const juce::var& json)
{
    auto id = str(json, "analysis_id");
    if (id.isNotEmpty())
        return id;
    if (auto* analysis = obj(json.getProperty("analysis", {})))
        return analysis->getProperty("id").toString();
    return str(json, "id");
}

juce::String parseAnalysisStatus(const juce::var& json)
{
    auto status = str(json, "status");
    if (status.isNotEmpty() && status != "ok")
        return status;
    if (auto* analysis = obj(json.getProperty("analysis", {})))
        return analysis->getProperty("status").toString();
    return status;
}

bool parseCompletedAnalysis(
    const juce::var& json,
    models::AudioAnalysis& analysis,
    std::vector<models::Issue>& issues)
{
    auto* analysisObj = obj(json.getProperty("analysis", {}));
    if (analysisObj == nullptr)
        return false;
    if (analysisObj->getProperty("status").toString() != "completed")
        return false;

    auto featuresVar = analysisObj->getProperty("features");
    auto* features = obj(featuresVar);
    if (features == nullptr)
        return false;

    analysis.analyzerVersion = features->getProperty("analyzer_version").toString().toStdString();
    analysis.bpm = (float) features->getProperty("bpm");
    analysis.durationSec = (float) features->getProperty("duration_sec");
    analysis.sampleRate = (int) features->getProperty("sample_rate");
    analysis.channels = (int) features->getProperty("channels");
    analysis.loudnessLufsApprox = (float) features->getProperty("loudness_lufs_approx");
    analysis.dynamicRangeDb = (float) features->getProperty("dynamic_range_db");
    analysis.stereoWidth = (float) features->getProperty("stereo_width");

    if (auto* key = obj(features->getProperty("key_estimation")))
    {
        const auto keyName = key->getProperty("key").toString();
        if (keyName.isNotEmpty() && keyName != "null")
            analysis.key.key = keyName.toStdString();
        analysis.key.confidence = (float) key->getProperty("confidence");
        analysis.key.method = key->getProperty("method").toString().toStdString();
    }

    if (auto* bands = obj(features->getProperty("frequency_distribution")))
    {
        analysis.bands.sub = (float) bands->getProperty("sub");
        analysis.bands.low = (float) bands->getProperty("low");
        analysis.bands.mid = (float) bands->getProperty("mid");
        analysis.bands.high = (float) bands->getProperty("high");
        analysis.bands.air = (float) bands->getProperty("air");
    }

    if (auto* dnaObj = obj(features->getProperty("dna")))
    {
        analysis.hasDna = true;
        auto& dna = analysis.dna;
        if (auto* identity = obj(dnaObj->getProperty("identity")))
        {
            dna.tempo = (float) identity->getProperty("tempo");
            if (auto* key = obj(identity->getProperty("key")))
            {
                const auto keyName = key->getProperty("name").toString();
                if (keyName.isNotEmpty() && keyName != "null")
                    dna.keyName = keyName.toStdString();
                dna.keyConfidence = (float) key->getProperty("confidence");
            }
            if (auto* genres = identity->getProperty("genre_profile").getArray())
            {
                for (const auto& item : *genres)
                    dna.genreProfile.push_back(item.toString().toStdString());
            }
        }
        if (auto* energy = obj(dnaObj->getProperty("energy")))
        {
            dna.energyMean = (float) energy->getProperty("mean");
            dna.energyPeak = (float) energy->getProperty("peak");
            if (auto* curve = energy->getProperty("curve").getArray())
            {
                for (const auto& item : *curve)
                    dna.energyCurve.push_back((float) item);
            }
            if (auto* peaks = energy->getProperty("peaks").getArray())
            {
                for (const auto& item : *peaks)
                    dna.energyPeaks.push_back((float) item);
            }
        }
        if (auto* structure = obj(dnaObj->getProperty("structure")))
        {
            if (auto* sections = structure->getProperty("sections").getArray())
            {
                for (const auto& item : *sections)
                {
                    if (auto* section = obj(item))
                    {
                        models::StructureSection row;
                        row.name = section->getProperty("name").toString().toStdString();
                        row.start = (float) section->getProperty("start");
                        row.end = (float) section->getProperty("end");
                        row.energy = (float) section->getProperty("energy");
                        row.bassEnergy = (float) section->getProperty("bass_energy");
                        row.transientDensity = (float) section->getProperty("transient_density");
                        row.stereoWidth = (float) section->getProperty("stereo_width");
                        dna.sections.push_back(std::move(row));
                    }
                }
            }
        }

        auto readAxis = [](juce::DynamicObject* axis, models::MixAxis& dest) {
            if (axis == nullptr)
                return;
            dest.sub = (float) axis->getProperty("sub");
            dest.low = (float) axis->getProperty("low");
            dest.control = (float) axis->getProperty("control");
            dest.value = (float) axis->getProperty("value");
            dest.risk = axis->getProperty("risk").toString().toStdString();
            dest.why = axis->getProperty("why").toString().toStdString();
            dest.method = axis->getProperty("method").toString().toStdString();
        };
        if (auto* mix = obj(dnaObj->getProperty("mix_character")))
        {
            readAxis(obj(mix->getProperty("low_end")), dna.lowEnd);
            readAxis(obj(mix->getProperty("brightness")), dna.brightness);
            readAxis(obj(mix->getProperty("stereo")), dna.stereo);
            readAxis(obj(mix->getProperty("dynamics")), dna.dynamics);
        }
        if (auto* translation = obj(dnaObj->getProperty("translation")))
        {
            if (auto* targets = translation->getProperty("targets").getArray())
            {
                for (const auto& item : *targets)
                {
                    if (auto* target = obj(item))
                    {
                        models::TranslationTarget row;
                        row.name = target->getProperty("name").toString().toStdString();
                        row.score = (float) target->getProperty("score");
                        row.issue = target->getProperty("issue").toString().toStdString();
                        row.reason = target->getProperty("reason").toString().toStdString();
                        row.action = target->getProperty("action").toString().toStdString();
                        if (row.issue == "null")
                            row.issue.clear();
                        if (row.reason == "null")
                            row.reason.clear();
                        if (row.action == "null")
                            row.action.clear();
                        dna.translation.push_back(std::move(row));
                    }
                }
            }
        }
        if (auto* masking = obj(dnaObj->getProperty("masking")))
        {
            if (auto* roles = masking->getProperty("roles").getArray())
            {
                for (const auto& item : *roles)
                    dna.maskingRoles.push_back(item.toString().toStdString());
            }
            if (auto* matrix = masking->getProperty("matrix").getArray())
            {
                for (const auto& rowVar : *matrix)
                {
                    std::vector<float> row;
                    if (auto* cells = rowVar.getArray())
                    {
                        for (const auto& cell : *cells)
                            row.push_back((float) cell);
                    }
                    dna.maskingMatrix.push_back(std::move(row));
                }
            }
        }
        if (auto* objects = dnaObj->getProperty("objects").getArray())
        {
            for (const auto& item : *objects)
            {
                if (auto* object = obj(item))
                {
                    models::SonoraObject row;
                    row.type = object->getProperty("type").toString().toStdString();
                    row.input = object->getProperty("input").toString().toStdString();
                    row.status = object->getProperty("status").toString().toStdString();
                    if (auto* parameters = obj(object->getProperty("parameters")))
                    {
                        row.frequency = (float) parameters->getProperty("frequency");
                        row.gain = (float) parameters->getProperty("gain");
                        row.q = (float) parameters->getProperty("q");
                    }
                    dna.objects.push_back(std::move(row));
                }
            }
        }
    }

    issues.clear();
    if (auto* list = analysisObj->getProperty("issues").getArray())
    {
        for (const auto& item : *list)
        {
            if (auto* issue = obj(item))
            {
                models::Issue row;
                row.type = issue->getProperty("type").toString().toStdString();
                row.severity = (float) issue->getProperty("severity");
                row.area = issue->getProperty("area").toString().toStdString();
                row.detail = issue->getProperty("detail").toString().toStdString();
                issues.push_back(std::move(row));
            }
        }
    }
    juce::ignoreUnused(num);
    return true;
}

std::vector<models::Insight> parseInsights(const juce::var& json)
{
    std::vector<models::Insight> out;
    if (auto* list = arr(json, "items"))
    {
        for (const auto& item : *list)
        {
            auto* o = obj(item);
            if (o == nullptr)
                continue;
            models::Insight insight;
            insight.issue = o->getProperty("target").toString().toStdString();
            insight.reason = o->getProperty("rationale").toString().toStdString();
            insight.action = o->getProperty("action").toString().toStdString();
            insight.operation = "Dynamic EQ";
            const auto hz = o->getProperty("frequency_hz");
            if (!hz.isVoid() && !hz.isUndefined())
                insight.frequencyHz = (float) hz;
            insight.priority = (int) o->getProperty("priority");
            insight.confidence = 0.82f;
            out.push_back(std::move(insight));
        }
    }
    return out;
}

models::Harmony parseHarmony(const juce::var& json)
{
    models::Harmony harmony;
    harmony.key = str(json, "key").toStdString();
    harmony.bars = (int) json.getProperty("bars", 8);
    if (auto* list = arr(json, "chords"))
    {
        for (const auto& item : *list)
            harmony.chords.push_back(item.toString().toStdString());
    }
    return harmony;
}

models::MidiClip parseMidiClip(const juce::var& json)
{
    models::MidiClip clip;
    clip.role = str(json, "role").toStdString();
    clip.key = str(json, "key").toStdString();
    clip.bars = (int) json.getProperty("bars", 8);
    if (auto* list = arr(json, "chords"))
    {
        for (const auto& item : *list)
            clip.chords.push_back(item.toString().toStdString());
    }
    if (auto* list = arr(json, "notes"))
    {
        for (const auto& item : *list)
            clip.notes.push_back(item.toString().toStdString());
    }
    if (auto* list = arr(json, "pattern"))
    {
        for (const auto& item : *list)
            clip.pattern.push_back(item.toString().toStdString());
    }
    return clip;
}

models::DropPlan parseDropPlan(const juce::var& json)
{
    models::DropPlan plan;
    plan.sectionName = str(json, "section_name").toStdString();
    plan.start = (float) json.getProperty("start", 0.0);
    plan.end = (float) json.getProperty("end", 0.0);
    plan.frequency = (float) json.getProperty("frequency", 3000.0);
    plan.gain = (float) json.getProperty("gain", 2.5);
    plan.q = (float) json.getProperty("q", 1.1);
    plan.energyTarget = (float) json.getProperty("energy_target", 0.85);
    if (auto* list = arr(json, "actions"))
    {
        for (const auto& item : *list)
            plan.actions.push_back(item.toString().toStdString());
    }
    return plan;
}

models::AssistAdvice parseAssist(const juce::var& json)
{
    models::AssistAdvice advice;
    advice.headline = str(json, "headline").toStdString();
    advice.detail = str(json, "detail").toStdString();
    advice.provider = str(json, "provider").toStdString();
    if (auto* list = arr(json, "options"))
    {
        for (const auto& item : *list)
        {
            auto* option = obj(item);
            if (option == nullptr)
                continue;
            models::AssistOption row;
            row.id = option->getProperty("id").toString().toStdString();
            row.label = option->getProperty("label").toString().toStdString();
            if (!row.id.empty())
                advice.options.push_back(std::move(row));
        }
    }
    return advice;
}

models::ReferenceReport parseReference(const juce::var& json)
{
    models::ReferenceReport report;
    report.targetFilename = str(json, "target_filename").toStdString();
    report.referenceFilename = str(json, "reference_filename").toStdString();
    if (auto* gap = obj(json.getProperty("gap", {})))
    {
        report.gap.loudnessLufs = (float) gap->getProperty("loudness_lufs");
        report.gap.lowEnd = (float) gap->getProperty("low_end");
        report.gap.stereo = (float) gap->getProperty("stereo");
        report.gap.brightness = (float) gap->getProperty("brightness");
    }
    if (auto* list = arr(json, "notes"))
    {
        for (const auto& item : *list)
            report.notes.push_back(item.toString().toStdString());
    }
    return report;
}

models::SessionProfile parseProfile(const juce::var& json)
{
    models::SessionProfile profile;
    if (auto* list = arr(json, "style"))
    {
        for (const auto& item : *list)
            profile.style.push_back(item.toString().toStdString());
    }
    if (auto* list = arr(json, "favorite_genres"))
    {
        for (const auto& item : *list)
            profile.genres.push_back(item.toString().toStdString());
    }
    return profile;
}

juce::String parseError(const juce::var& json)
{
    if (auto* analysis = obj(json.getProperty("analysis", {})))
    {
        const auto nested = analysis->getProperty("error").toString();
        if (nested.isNotEmpty())
            return nested;
    }
    auto message = str(json, "message");
    if (message.isNotEmpty())
        return message;
    return str(json, "detail");
}

} // namespace sonora::dto
