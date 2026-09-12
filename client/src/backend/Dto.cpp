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
