#include "engine/Engine.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <utility>

namespace sonora::engine
{
namespace
{
constexpr int kFftOrder = 11;
constexpr int kFftSize = 1 << kFftOrder;
constexpr int kSpecHop = 512;
constexpr double kAnalyzeSr = 22050.0;
constexpr float kMaxSeconds = 480.0f;
constexpr float kPi = 3.14159265358979323846f;

const char* kPitches[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
const float kMajor[] = { 6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f };
const float kMinor[] = { 6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f };

float clip01(float v) { return juce::jlimit(0.0f, 1.0f, v); }

float meanSq(const float* data, int n)
{
    if (n <= 0)
        return 0.0f;
    double acc = 0.0;
    for (int i = 0; i < n; ++i)
        acc += (double) data[i] * (double) data[i];
    return (float) (acc / (double) n);
}

void toMono(const juce::AudioBuffer<float>& src, std::vector<float>& mono)
{
    const int n = src.getNumSamples();
    const int ch = src.getNumChannels();
    mono.resize((size_t) n);
    if (n == 0)
        return;
    if (ch <= 1)
    {
        const float* p = src.getReadPointer(0);
        std::copy(p, p + n, mono.begin());
        return;
    }
    const float* L = src.getReadPointer(0);
    const float* R = src.getReadPointer(1);
    for (int i = 0; i < n; ++i)
        mono[(size_t) i] = 0.5f * (L[i] + R[i]);
}

juce::AudioBuffer<float> downsample(const juce::AudioBuffer<float>& src, double srcSr, double dstSr, int maxSamples)
{
    const int srcN = src.getNumSamples();
    const int ch = juce::jmax(1, src.getNumChannels());
    if (srcN <= 0 || srcSr <= 0.0)
        return {};

    const double ratio = dstSr / srcSr;
    int dstN = juce::jmax(1, (int) std::llround((double) srcN * ratio));
    dstN = juce::jmin(dstN, maxSamples);
    juce::AudioBuffer<float> out(juce::jmin(ch, 2), dstN);
    out.clear();
    for (int c = 0; c < out.getNumChannels(); ++c)
    {
        const int srcCh = juce::jmin(c, src.getNumChannels() - 1);
        const float* in = src.getReadPointer(srcCh);
        float* dest = out.getWritePointer(c);
        for (int i = 0; i < dstN; ++i)
        {
            const double srcPos = (double) i / ratio;
            const int i0 = juce::jlimit(0, srcN - 1, (int) srcPos);
            const int i1 = juce::jmin(srcN - 1, i0 + 1);
            const float t = (float) (srcPos - (double) i0);
            dest[i] = in[i0] + (in[i1] - in[i0]) * t;
        }
    }
    return out;
}

void minmaxInPlace(std::vector<float>& v)
{
    if (v.empty())
        return;
    auto mm = std::minmax_element(v.begin(), v.end());
    const float lo = *mm.first;
    const float hi = *mm.second;
    if (hi - lo < 1e-9f)
    {
        std::fill(v.begin(), v.end(), 0.5f);
        return;
    }
    for (auto& x : v)
        x = (x - lo) / (hi - lo);
}

std::vector<float> smooth(const std::vector<float>& v, int window)
{
    const int size = juce::jmax(1, window);
    if (size <= 1 || v.empty())
        return v;
    std::vector<float> out(v.size(), 0.0f);
    const float inv = 1.0f / (float) size;
    for (int i = 0; i < (int) v.size(); ++i)
    {
        float acc = 0.0f;
        int count = 0;
        for (int k = i - size / 2; k <= i + size / 2; ++k)
        {
            if (k >= 0 && k < (int) v.size())
            {
                acc += v[(size_t) k];
                ++count;
            }
        }
        out[(size_t) i] = count > 0 ? acc / (float) count : v[(size_t) i];
        juce::ignoreUnused(inv);
    }
    return out;
}

struct Spectrum
{
    std::vector<float> meanMag;
    std::vector<float> freqs;
};

Spectrum meanSpectrum(const std::vector<float>& mono, double sr)
{
    Spectrum spec;
    spec.meanMag.assign(kFftSize / 2 + 1, 0.0f);
    spec.freqs.resize(kFftSize / 2 + 1);
    for (int i = 0; i < (int) spec.freqs.size(); ++i)
        spec.freqs[(size_t) i] = (float) (i * sr / (double) kFftSize);

    if (mono.empty())
        return spec;

    juce::dsp::FFT fft(kFftOrder);
    std::vector<float> window((size_t) kFftSize);
    for (int i = 0; i < kFftSize; ++i)
        window[(size_t) i] = 0.5f - 0.5f * std::cos(2.0f * kPi * (float) i / (float) (kFftSize - 1));

    std::vector<float> work((size_t) kFftSize * 2, 0.0f);
    int frames = 0;
    for (int start = 0; start + kFftSize / 4 < (int) mono.size(); start += kSpecHop)
    {
        std::fill(work.begin(), work.end(), 0.0f);
        const int n = juce::jmin(kFftSize, (int) mono.size() - start);
        for (int i = 0; i < n; ++i)
            work[(size_t) i] = mono[(size_t) (start + i)] * window[(size_t) i];
        fft.performRealOnlyForwardTransform(work.data());
        spec.meanMag[0] += std::abs(work[0]);
        spec.meanMag[(size_t) (kFftSize / 2)] += std::abs(work[1]);
        for (int bin = 1; bin < kFftSize / 2; ++bin)
            spec.meanMag[(size_t) bin] += std::hypot(work[(size_t) (2 * bin)], work[(size_t) (2 * bin + 1)]);
        ++frames;
    }
    if (frames > 0)
        for (auto& m : spec.meanMag)
            m /= (float) frames;
    return spec;
}

float bandEnergy(const Spectrum& spec, float lowHz, float highHz)
{
    if (spec.freqs.empty())
        return 0.0f;
    const float nyq = spec.freqs.back();
    const float hi = juce::jmin(highHz, nyq);
    double acc = 0.0;
    for (int i = 0; i < (int) spec.freqs.size(); ++i)
    {
        const float f = spec.freqs[(size_t) i];
        if (f >= lowHz && f < hi)
        {
            const float m = spec.meanMag[(size_t) i];
            acc += (double) m * (double) m;
        }
    }
    return (float) acc;
}

models::FrequencyDistribution distribution(const Spectrum& spec)
{
    const float sub = bandEnergy(spec, 20.0f, 60.0f);
    const float low = bandEnergy(spec, 60.0f, 250.0f);
    const float mid = bandEnergy(spec, 250.0f, 2000.0f);
    const float high = bandEnergy(spec, 2000.0f, 8000.0f);
    const float air = bandEnergy(spec, 8000.0f, 20000.0f);
    const float total = sub + low + mid + high + air + 1e-12f;
    return { sub / total, low / total, mid / total, high / total, air / total };
}

float centroidHz(const Spectrum& spec)
{
    double num = 0.0, den = 0.0;
    for (int i = 0; i < (int) spec.freqs.size(); ++i)
    {
        const double m = spec.meanMag[(size_t) i];
        num += spec.freqs[(size_t) i] * m;
        den += m;
    }
    return den > 1e-12 ? (float) (num / den) : 0.0f;
}

float rolloffHz(const Spectrum& spec)
{
    double total = 0.0;
    for (float m : spec.meanMag)
        total += m;
    double acc = 0.0;
    const double target = 0.85 * total;
    for (int i = 0; i < (int) spec.freqs.size(); ++i)
    {
        acc += spec.meanMag[(size_t) i];
        if (acc >= target)
            return spec.freqs[(size_t) i];
    }
    return spec.freqs.empty() ? 0.0f : spec.freqs.back();
}

float stereoWidth(const juce::AudioBuffer<float>& buf)
{
    if (buf.getNumChannels() < 2 || buf.getNumSamples() <= 0)
        return 0.0f;
    const float* L = buf.getReadPointer(0);
    const float* R = buf.getReadPointer(1);
    double mid = 0.0, side = 0.0;
    const int n = buf.getNumSamples();
    for (int i = 0; i < n; ++i)
    {
        const double m = 0.5 * ((double) L[i] + (double) R[i]);
        const double s = 0.5 * ((double) L[i] - (double) R[i]);
        mid += m * m;
        side += s * s;
    }
    mid /= n;
    side /= n;
    return (float) (side / (mid + side + 1e-12));
}

float peakAmp(const juce::AudioBuffer<float>& buf)
{
    return buf.getMagnitude(0, buf.getNumSamples());
}

float rmsAmp(const std::vector<float>& mono)
{
    if (mono.empty())
        return 0.0f;
    return std::sqrt(meanSq(mono.data(), (int) mono.size()) + 1e-12f);
}

void biquad(std::vector<float>& x, const float* b, const float* a)
{
    float z1 = 0.0f, z2 = 0.0f;
    for (float& s : x)
    {
        const float y = b[0] * s + z1;
        z1 = b[1] * s - a[1] * y + z2;
        z2 = b[2] * s - a[2] * y;
        s = y;
    }
}

float loudnessApprox(const juce::AudioBuffer<float>& buf, double sr)
{
    auto at48 = downsample(buf, sr, 48000.0, (int) (48000.0 * kMaxSeconds));
    if (at48.getNumSamples() <= 0)
        return -70.0f;
    const float bShelf[] = { 1.53512486f, -2.69169619f, 1.19839281f };
    const float aShelf[] = { 1.0f, -1.69065929f, 0.73248077f };
    const float bHp[] = { 1.0f, -2.0f, 1.0f };
    const float aHp[] = { 1.0f, -1.99004745f, 0.99007225f };
    const float gains[] = { 1.0f, 1.0f };
    double total = 0.0;
    const int ch = juce::jmin(2, at48.getNumChannels());
    for (int c = 0; c < ch; ++c)
    {
        std::vector<float> x((size_t) at48.getNumSamples());
        std::copy(at48.getReadPointer(c), at48.getReadPointer(c) + at48.getNumSamples(), x.begin());
        biquad(x, bShelf, aShelf);
        biquad(x, bHp, aHp);
        total += (double) gains[c] * meanSq(x.data(), (int) x.size());
    }
    return (float) (-0.691 + 10.0 * std::log10(total + 1e-12));
}

float estimateBpm(const std::vector<float>& onset, double hopSec)
{
    if (onset.size() < 16 || hopSec <= 0.0)
        return 0.0f;
    const int minLag = juce::jmax(2, (int) std::llround((60.0 / 180.0) / hopSec));
    const int maxLag = juce::jmin((int) onset.size() / 2, (int) std::llround((60.0 / 70.0) / hopSec));
    if (maxLag <= minLag)
        return 0.0f;

    float best = 0.0f;
    int bestLag = minLag;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double acc = 0.0;
        const int n = (int) onset.size() - lag;
        for (int i = 0; i < n; ++i)
            acc += (double) onset[(size_t) i] * (double) onset[(size_t) (i + lag)];
        const float score = (float) (acc / (double) juce::jmax(1, n));
        if (score > best)
        {
            best = score;
            bestLag = lag;
        }
    }
    const float bpm = (float) (60.0 / (hopSec * (double) bestLag));
    if (!std::isfinite(bpm) || bpm < 60.0f || bpm > 200.0f)
        return 0.0f;
    return std::round(bpm * 100.0f) / 100.0f;
}

void estimateKey(const Spectrum& spec, models::KeyEstimation& key)
{
    key.method = "chroma_stft";
    key.confidence = 0.0f;
    key.key.reset();
    float chroma[12] = {};
    for (int i = 1; i < (int) spec.freqs.size(); ++i)
    {
        const float f = spec.freqs[(size_t) i];
        if (f < 40.0f || f > 5000.0f)
            continue;
        const float midi = 69.0f + 12.0f * std::log2(f / 440.0f);
        int pc = (int) std::lround(midi) % 12;
        if (pc < 0)
            pc += 12;
        chroma[pc] += spec.meanMag[(size_t) i];
    }
    float norm = 0.0f;
    for (float c : chroma)
        norm += c * c;
    norm = std::sqrt(norm);
    if (norm < 1e-9f)
        return;
    for (float& c : chroma)
        c /= norm;

    auto corr = [](const float* a, const float* prof, int shift) {
        float s = 0.0f;
        for (int i = 0; i < 12; ++i)
            s += a[i] * prof[(i - shift + 12) % 12];
        return s;
    };
    float majorN = 0.0f, minorN = 0.0f;
    for (int i = 0; i < 12; ++i)
    {
        majorN += kMajor[i] * kMajor[i];
        minorN += kMinor[i] * kMinor[i];
    }
    majorN = std::sqrt(majorN);
    minorN = std::sqrt(minorN);
    float majorP[12], minorP[12];
    for (int i = 0; i < 12; ++i)
    {
        majorP[i] = kMajor[i] / majorN;
        minorP[i] = kMinor[i] / minorN;
    }

    std::vector<std::pair<float, std::string>> scores;
    scores.reserve(24);
    for (int shift = 0; shift < 12; ++shift)
    {
        scores.push_back({ corr(chroma, majorP, shift), std::string(kPitches[shift]) + " major" });
        scores.push_back({ corr(chroma, minorP, shift), std::string(kPitches[shift]) + " minor" });
    }
    std::sort(scores.begin(), scores.end(), [](auto& a, auto& b) { return a.first > b.first; });
    const float best = scores[0].first;
    const float second = scores[1].first;
    const float gap = juce::jmax(0.0f, best - second);
    key.confidence = clip01(juce::jmax(0.0f, best) * (0.45f + gap));
    if (best > 0.35f && key.confidence >= 0.20f)
        key.key = scores[0].second;
}

std::vector<float> peakEnvelope(const std::vector<float>& mono, int bins = 512)
{
    std::vector<float> out;
    if (mono.empty())
        return std::vector<float>(bins, 0.0f);
    float peak = 0.0f;
    for (float s : mono)
        peak = juce::jmax(peak, std::abs(s));
    peak += 1e-12f;
    if ((int) mono.size() <= bins)
    {
        out.reserve(mono.size());
        for (float s : mono)
            out.push_back(std::round(std::abs(s) / peak * 10000.0f) / 10000.0f);
        return out;
    }
    out.resize((size_t) bins);
    for (int i = 0; i < bins; ++i)
    {
        const int start = (int) ((int64_t) i * (int) mono.size() / bins);
        const int end = juce::jmax(start + 1, (int) ((int64_t) (i + 1) * (int) mono.size() / bins));
        float m = 0.0f;
        for (int k = start; k < end && k < (int) mono.size(); ++k)
            m = juce::jmax(m, std::abs(mono[(size_t) k]));
        out[(size_t) i] = std::round(m / peak * 10000.0f) / 10000.0f;
    }
    return out;
}

std::vector<float> downsampleCurve(std::vector<float> values, int bins = 48)
{
    minmaxInPlace(values);
    if (values.empty())
        return std::vector<float>(bins, 0.0f);
    if ((int) values.size() <= bins)
        return values;
    std::vector<float> out((size_t) bins);
    for (int i = 0; i < bins; ++i)
    {
        const int start = (int) ((int64_t) i * (int) values.size() / bins);
        const int end = juce::jmax(start + 1, (int) ((int64_t) (i + 1) * (int) values.size() / bins));
        float acc = 0.0f;
        int n = 0;
        for (int k = start; k < end && k < (int) values.size(); ++k)
        {
            acc += values[(size_t) k];
            ++n;
        }
        out[(size_t) i] = std::round((n > 0 ? acc / (float) n : 0.0f) * 10000.0f) / 10000.0f;
    }
    return out;
}

struct Timeline
{
    float hopSec = 0.25f;
    std::vector<float> times;
    std::vector<float> rms;
    std::vector<float> flux;
    std::vector<float> transient;
    std::vector<float> bassShare;
    std::vector<float> width;
};

Timeline computeTimeline(const juce::AudioBuffer<float>& buf, const std::vector<float>& mono, double sr)
{
    Timeline tl;
    const int hop = juce::jmax(1, (int) std::llround(sr * 0.25));
    tl.hopSec = (float) hop / (float) sr;
    if (mono.empty())
        return tl;

    juce::dsp::FFT fft(kFftOrder);
    std::vector<float> window((size_t) kFftSize);
    for (int i = 0; i < kFftSize; ++i)
        window[(size_t) i] = 0.5f - 0.5f * std::cos(2.0f * kPi * (float) i / (float) (kFftSize - 1));

    std::vector<float> work((size_t) kFftSize * 2, 0.0f);
    std::vector<float> prev((size_t) (kFftSize / 2 + 1), 0.0f);
    const int nFrames = juce::jmax(1, ((int) mono.size() + hop - 1) / hop);
    tl.times.resize((size_t) nFrames);
    tl.rms.resize((size_t) nFrames);
    tl.flux.resize((size_t) nFrames);
    tl.transient.resize((size_t) nFrames);
    tl.bassShare.resize((size_t) nFrames);
    tl.width.resize((size_t) nFrames);

    for (int f = 0; f < nFrames; ++f)
    {
        const int center = f * hop;
        tl.times[(size_t) f] = (float) center / (float) sr;
        const int start = juce::jmax(0, center - kFftSize / 2);
        std::fill(work.begin(), work.end(), 0.0f);
        for (int i = 0; i < kFftSize; ++i)
        {
            const int idx = start + i;
            if (idx >= 0 && idx < (int) mono.size())
                work[(size_t) i] = mono[(size_t) idx] * window[(size_t) i];
        }
        float frameRms = 0.0f;
        for (int i = 0; i < kFftSize; ++i)
            frameRms += work[(size_t) i] * work[(size_t) i];
        tl.rms[(size_t) f] = std::sqrt(frameRms / (float) kFftSize + 1e-12f);

        fft.performRealOnlyForwardTransform(work.data());
        std::vector<float> mag((size_t) (kFftSize / 2 + 1));
        mag[0] = std::abs(work[0]);
        mag[(size_t) (kFftSize / 2)] = std::abs(work[1]);
        for (int bin = 1; bin < kFftSize / 2; ++bin)
            mag[(size_t) bin] = std::hypot(work[(size_t) (2 * bin)], work[(size_t) (2 * bin + 1)]);

        double flux = 0.0, bass = 0.0, total = 0.0;
        for (int bin = 0; bin < (int) mag.size(); ++bin)
        {
            const float d = mag[(size_t) bin] - prev[(size_t) bin];
            if (d > 0.0f)
                flux += (double) d * (double) d;
            const float hz = (float) (bin * sr / (double) kFftSize);
            const float e = mag[(size_t) bin] * mag[(size_t) bin];
            total += e;
            if (hz >= 20.0f && hz < 250.0f)
                bass += e;
        }
        tl.flux[(size_t) f] = (float) std::sqrt(flux);
        tl.transient[(size_t) f] = tl.flux[(size_t) f];
        tl.bassShare[(size_t) f] = (float) (bass / (total + 1e-12));
        prev.swap(mag);

        if (buf.getNumChannels() >= 2)
        {
            const int a = juce::jmax(0, center - kFftSize / 2);
            const int b = juce::jmin(buf.getNumSamples(), a + kFftSize);
            const float* L = buf.getReadPointer(0);
            const float* R = buf.getReadPointer(1);
            double mid = 0.0, side = 0.0;
            for (int i = a; i < b; ++i)
            {
                const double m = 0.5 * ((double) L[i] + (double) R[i]);
                const double s = 0.5 * ((double) L[i] - (double) R[i]);
                mid += m * m;
                side += s * s;
            }
            tl.width[(size_t) f] = (float) (side / (mid + side + 1e-12));
        }
    }
    tl.transient = tl.flux;
    minmaxInPlace(tl.transient);
    return tl;
}

std::vector<int> findBoundaries(const std::vector<float>& novelty, float hopSec, float duration, float bpm)
{
    const int n = (int) novelty.size();
    if (n <= 2)
        return { 0, juce::jmax(1, n) };
    const float beat = bpm >= 60.0f ? 60.0f / bpm : 0.5f;
    const float minLen = duration < 45.0f ? juce::jmax(4.0f, duration / 6.0f) : juce::jmax(6.0f, juce::jmin(16.0f, 8.0f * beat));
    const int minFrames = juce::jmax(2, (int) std::lround(minLen / juce::jmax(hopSec, 1e-6f)));
    std::vector<int> peaks;
    for (int i = 1; i < n - 1; ++i)
    {
        if (novelty[(size_t) i] >= novelty[(size_t) (i - 1)] && novelty[(size_t) i] >= novelty[(size_t) (i + 1)]
            && novelty[(size_t) i] >= 0.06f)
        {
            if (peaks.empty() || i - peaks.back() >= minFrames)
                peaks.push_back(i);
        }
    }
    std::vector<int> merged { 0 };
    for (int p : peaks)
        if (p - merged.back() >= minFrames)
            merged.push_back(p);
    if (merged.back() != n)
        merged.push_back(n);
    if ((int) merged.size() == 2 && n > minFrames * 2)
        merged = { 0, n / 3, 2 * n / 3, n };
    return merged;
}

void labelSections(std::vector<models::StructureSection>& sections)
{
    if (sections.size() == 1)
    {
        sections[0].name = "body";
        return;
    }
    float maxE = 0.0f, minE = 1.0f;
    int dropI = 0;
    std::vector<float> energies;
    for (int i = 0; i < (int) sections.size(); ++i)
    {
        energies.push_back(sections[(size_t) i].energy);
        if (sections[(size_t) i].energy > maxE)
        {
            maxE = sections[(size_t) i].energy;
            dropI = i;
        }
        minE = juce::jmin(minE, sections[(size_t) i].energy);
    }
    std::vector<float> sorted = energies;
    std::sort(sorted.begin(), sorted.end());
    const float median = sorted[sorted.size() / 2];
    const float spread = maxE - minE;
    if (spread < 0.12f)
    {
        if (sections.size() == 2)
        {
            sections[0].name = "intro";
            sections[1].name = "outro";
            return;
        }
        sections[0].name = "intro";
        sections.back().name = "outro";
        for (size_t i = 1; i + 1 < sections.size(); ++i)
            sections[i].name = "groove";
        return;
    }
    for (int i = 0; i < (int) sections.size(); ++i)
    {
        const float e = sections[(size_t) i].energy;
        if (i == 0 && e <= median + 0.05f)
            sections[(size_t) i].name = "intro";
        else if (i == (int) sections.size() - 1 && e <= median + 0.08f)
            sections[(size_t) i].name = "outro";
        else if (i == dropI && e >= juce::jmax(0.55f, median + 0.08f))
            sections[(size_t) i].name = "drop";
        else if (i < dropI && energies[(size_t) dropI] - e >= 0.10f)
            sections[(size_t) i].name = (i + 1 == dropI || e >= sections[(size_t) juce::jmax(0, i - 1)].energy - 0.02f)
                                            ? "build"
                                            : "groove";
        else if (i > dropI && energies[(size_t) dropI] - e >= 0.15f)
            sections[(size_t) i].name = "break";
        else
            sections[(size_t) i].name = "groove";
    }
    bool hasDrop = false;
    for (const auto& s : sections)
        if (s.name == "drop")
            hasDrop = true;
    if (!hasDrop)
        sections[(size_t) dropI].name = "drop";
}

std::vector<models::StructureSection> detectStructure(const Timeline& tl, float duration, float bpm)
{
    if (tl.times.empty() || duration <= 0.0f)
    {
        models::StructureSection body;
        body.name = "body";
        body.end = duration;
        return { body };
    }
    auto energy = tl.rms;
    minmaxInPlace(energy);
    energy = smooth(energy, juce::jmax(3, (int) std::lround(1.0f / juce::jmax(tl.hopSec, 1e-6f))));
    auto flux = tl.flux;
    minmaxInPlace(flux);
    std::vector<float> novelty(energy.size());
    for (int i = 0; i < (int) energy.size(); ++i)
    {
        const float prev = i > 0 ? energy[(size_t) (i - 1)] : energy[(size_t) i];
        const float next = i + 1 < (int) energy.size() ? energy[(size_t) (i + 1)] : energy[(size_t) i];
        const float grad = std::abs(next - prev) * 0.5f;
        novelty[(size_t) i] = 0.65f * grad + 0.35f * flux[(size_t) i];
    }
    novelty = smooth(novelty, juce::jmax(3, (int) std::lround(0.75f / juce::jmax(tl.hopSec, 1e-6f))));

    std::vector<int> bounds = duration < 20.0f ? std::vector<int> { 0, (int) energy.size() }
                                               : findBoundaries(novelty, tl.hopSec, duration, bpm);
    std::vector<models::StructureSection> sections;
    for (int i = 0; i + 1 < (int) bounds.size(); ++i)
    {
        int a = bounds[(size_t) i];
        int b = juce::jmax(a + 1, bounds[(size_t) (i + 1)]);
        models::StructureSection row;
        row.name = "groove";
        row.start = juce::jmax(0.0f, tl.times[(size_t) a]);
        row.end = juce::jmin(duration, tl.times[(size_t) juce::jmin(b, (int) tl.times.size()) - 1] + tl.hopSec);
        auto meanRange = [](const std::vector<float>& v, int s, int e) {
            if (v.empty() || s >= e)
                return 0.0f;
            e = juce::jmin(e, (int) v.size());
            s = juce::jlimit(0, (int) v.size() - 1, s);
            float acc = 0.0f;
            int n = 0;
            for (int k = s; k < e; ++k)
            {
                acc += v[(size_t) k];
                ++n;
            }
            return clip01(n > 0 ? acc / (float) n : 0.0f);
        };
        row.energy = std::round(meanRange(energy, a, b) * 10000.0f) / 10000.0f;
        row.bassEnergy = std::round(meanRange(tl.bassShare, a, b) * 10000.0f) / 10000.0f;
        row.transientDensity = std::round(meanRange(tl.transient, a, b) * 10000.0f) / 10000.0f;
        row.stereoWidth = std::round(meanRange(tl.width, a, b) * 10000.0f) / 10000.0f;
        sections.push_back(row);
    }
    if (!sections.empty())
    {
        sections.front().start = 0.0f;
        sections.back().end = std::round(duration * 1000.0f) / 1000.0f;
    }
    labelSections(sections);
    return sections;
}

std::string riskOf(float level)
{
    if (level >= 0.62f)
        return "high";
    if (level >= 0.38f)
        return "medium";
    return "low";
}

void buildMix(models::TrackDna& dna, const Spectrum& spec, const models::AudioAnalysis& analysis)
{
    const auto dist = analysis.bands;
    const float mud = bandEnergy(spec, 80.0f, 120.0f);
    const float punch = bandEnergy(spec, 100.0f, 150.0f);
    const float subE = bandEnergy(spec, 20.0f, 60.0f);
    const float lowE = bandEnergy(spec, 60.0f, 250.0f);
    const float fineTotal = mud + punch + subE + lowE + bandEnergy(spec, 250.0f, 20000.0f) + 1e-12f;
    const float mudShare = mud / fineTotal;
    const float punchShare = punch / fineTotal;
    const float subShare = subE / fineTotal;
    const float lowEnd = dist.sub + dist.low;
    const float mudRatio = mudShare / (subShare + lowE / fineTotal + 1e-12f);

    dna.lowEnd.sub = std::round(clip01(dist.sub / 0.18f) * 1000.0f) / 1000.0f;
    dna.lowEnd.low = std::round(clip01(dist.low / 0.28f) * 1000.0f) / 1000.0f;
    dna.lowEnd.control = std::round(clip01(1.0f - mudRatio * 1.45f) * 1000.0f) / 1000.0f;
    dna.lowEnd.method = "band_share + 80_120_concentration";
    float riskLevel = lowEnd * 0.7f;
    if (mudRatio >= 0.28f)
    {
        dna.lowEnd.why = "Excess energy 80-120Hz";
        riskLevel = juce::jmax(lowEnd, mudRatio + 0.2f);
    }
    else if (dist.sub >= 0.14f && punchShare < 0.06f)
    {
        dna.lowEnd.why = "Sub energy dominates below 60Hz";
        riskLevel = juce::jmax(lowEnd, dist.sub + 0.25f);
    }
    else if (lowEnd >= 0.50f)
    {
        dna.lowEnd.why = "Low-end share is high relative to the rest of the spectrum";
        riskLevel = lowEnd;
    }
    else
        dna.lowEnd.why = "Low end is balanced";
    dna.lowEnd.risk = riskOf(riskLevel);

    const float brightness = clip01((dist.high + dist.air) / 0.28f);
    dna.brightness.value = std::round(brightness * 1000.0f) / 1000.0f;
    dna.brightness.method = "high+air share";
    if (brightness < 0.28f)
    {
        dna.brightness.why = "High-frequency energy is thin";
        dna.brightness.risk = brightness < 0.18f ? "medium" : "low";
    }
    else if (brightness > 0.85f)
    {
        dna.brightness.why = "Top end is aggressive relative to body";
        dna.brightness.risk = "medium";
    }
    else
    {
        dna.brightness.why = "Brightness is in range";
        dna.brightness.risk = "low";
    }

    dna.stereo.value = analysis.stereoWidth;
    dna.stereo.method = "side/(mid+side)";
    if (analysis.channels == 1)
    {
        dna.stereo.why = "Source is mono";
        dna.stereo.risk = "high";
        dna.stereo.value = 0.0f;
    }
    else if (analysis.stereoWidth < 0.08f)
    {
        dna.stereo.why = "Stereo width is narrow";
        dna.stereo.risk = "high";
    }
    else if (analysis.stereoWidth > 0.62f)
    {
        dna.stereo.why = "Side energy is unusually high";
        dna.stereo.risk = "medium";
    }
    else
    {
        dna.stereo.why = "Stereo image is usable";
        dna.stereo.risk = "low";
    }

    dna.dynamics.value = std::round(clip01(analysis.dynamicRangeDb / 16.0f) * 1000.0f) / 1000.0f;
    dna.dynamics.method = "crest_factor_db";
    if (analysis.dynamicRangeDb < 6.0f)
    {
        dna.dynamics.why = "Crest factor is tight";
        dna.dynamics.risk = "high";
    }
    else if (analysis.dynamicRangeDb < 8.0f)
    {
        dna.dynamics.why = "Crest factor is modest";
        dna.dynamics.risk = "medium";
    }
    else
    {
        dna.dynamics.why = "Dynamic range is usable";
        dna.dynamics.risk = "low";
    }
}

void buildTranslation(models::TrackDna& dna, const models::AudioAnalysis& analysis)
{
    struct W
    {
        const char* name;
        float sub, low, mid, high, air;
    };
    const W weights[] = {
        { "phone", 0.05f, 0.40f, 1.00f, 0.90f, 0.45f },
        { "car", 0.70f, 1.10f, 0.90f, 0.75f, 0.30f },
        { "club", 1.20f, 1.10f, 0.85f, 0.70f, 0.25f },
        { "headphones", 0.55f, 0.90f, 1.00f, 1.00f, 0.90f },
    };
    const float shares[] = { analysis.bands.sub, analysis.bands.low, analysis.bands.mid, analysis.bands.high, analysis.bands.air };
    const bool punchWeak = dna.lowEnd.sub >= 0.55f && dna.lowEnd.control < 0.45f;
    const bool subHeavy = analysis.bands.sub >= 0.12f;

    for (const auto& w : weights)
    {
        const float wt[] = { w.sub, w.low, w.mid, w.high, w.air };
        float lost = 0.0f, boom = 0.0f;
        for (int i = 0; i < 5; ++i)
        {
            if (wt[i] < 1.0f)
                lost += shares[i] * (1.0f - wt[i]);
            else
                boom += shares[i] * (wt[i] - 1.0f);
        }
        models::TranslationTarget t;
        t.name = w.name;
        t.score = std::round(clip01(1.0f - lost * 1.20f - boom * 0.35f) * 1000.0f) / 1000.0f;
        if (t.name == "phone" && (lost >= 0.22f || (subHeavy && punchWeak)))
        {
            t.score = juce::jmin(t.score, subHeavy ? 0.48f : t.score);
            t.issue = "Low end disappears on phone";
            t.reason = "Sub energy dominates below 60Hz";
            t.action = "Move bass information to 100-150Hz";
        }
        else if (t.name == "car" && boom >= 0.08f && analysis.bands.low >= 0.28f)
        {
            t.issue = "Low end blooms in a car";
            t.reason = "Cabin gain emphasizes 60-120Hz";
            t.action = "Tighten 80-120Hz before the sub";
        }
        else if (t.name == "club" && analysis.bands.sub >= 0.18f && analysis.bands.low < 0.16f)
        {
            t.issue = "Club system will feel hollow";
            t.reason = "Sub is present but punch band is thin";
            t.action = "Add harmonic bass information at 100-150Hz";
        }
        else if (t.name == "headphones" && (analysis.bands.high + analysis.bands.air) < 0.10f)
        {
            t.issue = "Headphones expose a dull top";
            t.reason = "High/air energy share is low";
            t.action = "Open 6-10kHz after the mix body is stable";
        }
        else if (t.name == "headphones" && dna.stereo.risk == "high")
        {
            t.issue = "Headphones collapse the image";
            t.reason = dna.stereo.why;
            t.action = "Restore mid/side width above 400Hz";
        }
        dna.translation.push_back(std::move(t));
    }
}

void buildMasking(models::TrackDna& dna, const Spectrum& spec)
{
    dna.maskingRoles = { "kick", "bass", "vocal", "lead" };
    const float bands[][2] = { { 40.0f, 90.0f }, { 70.0f, 200.0f }, { 300.0f, 3500.0f }, { 2000.0f, 8000.0f } };
    float roleE[4] = {};
    float total = 1e-12f;
    for (int i = 0; i < 4; ++i)
    {
        roleE[i] = bandEnergy(spec, bands[i][0], bands[i][1]);
        total += roleE[i];
    }
    const float overlap[4][4][2] = {
        { { 0, 0 }, { 70, 90 }, { 300, 400 }, { 2000, 2500 } },
        { { 70, 90 }, { 0, 0 }, { 250, 400 }, { 2000, 2500 } },
        { { 300, 400 }, { 250, 400 }, { 0, 0 }, { 2000, 3500 } },
        { { 2000, 2500 }, { 2000, 2500 }, { 2000, 3500 }, { 0, 0 } },
    };
    dna.maskingMatrix.assign(4, std::vector<float>(4, 0.0f));
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            if (r == c)
                continue;
            const float ov = bandEnergy(spec, overlap[r][c][0], overlap[r][c][1]);
            const float pair = roleE[r] + roleE[c] + 1e-12f;
            const float presence = juce::jmin(roleE[r], roleE[c]) / total;
            dna.maskingMatrix[(size_t) r][(size_t) c] =
                std::round(clip01((2.0f * ov / pair) * (0.35f + 2.4f * presence)) * 1000.0f) / 1000.0f;
        }
    }
}

std::vector<std::string> inferGenre(float bpm, float duration, const models::FrequencyDistribution& dist, float transientMean)
{
    if (duration < 15.0f || bpm < 70.0f)
        return {};
    std::vector<std::string> tags;
    const float lowEnd = dist.sub + dist.low;
    if (bpm >= 118.0f && bpm <= 132.0f)
    {
        tags.emplace_back("house");
        if (lowEnd >= 0.42f && dist.high < 0.22f)
            tags.emplace_back("deep house");
        if (transientMean >= 0.42f && dist.high >= 0.12f)
            tags.emplace_back("slap house");
        if (dist.mid >= 0.34f && bpm >= 124.0f && bpm <= 130.0f)
            tags.emplace_back("tech house");
    }
    else if (bpm > 132.0f && bpm <= 148.0f && lowEnd >= 0.36f)
        tags.emplace_back("techno");
    else if (bpm >= 168.0f && bpm <= 180.0f)
        tags.emplace_back("drum and bass");
    else if (bpm >= 80.0f && bpm <= 100.0f && lowEnd >= 0.40f)
        tags.emplace_back("hip hop");
    else if (bpm > 0.0f)
        tags.emplace_back("electronic");
    if (tags.size() > 3)
        tags.resize(3);
    return tags;
}

std::vector<models::Issue> detectIssues(const models::AudioAnalysis& analysis)
{
    std::vector<models::Issue> issues;
    const auto& d = analysis.bands;
    if (d.sub + d.low >= 0.50f)
        issues.push_back({ "muddy_low_end", clip01((d.sub + d.low - 0.35f) / 0.45f), "low_end", "Sub+low energy share is high" });
    if (d.low >= 0.26f && d.mid >= 0.26f)
        issues.push_back({ "frequency_conflict", clip01(juce::jmin(d.low, d.mid) / 0.40f), "low_end", "Low and mid bands both carry high energy share" });
    if (analysis.channels == 1 || analysis.stereoWidth < 0.08f)
        issues.push_back({ "narrow_stereo",
                           analysis.channels == 1 ? 0.85f : clip01((0.08f - analysis.stereoWidth) / 0.08f),
                           "stereo_image",
                           analysis.channels == 1 ? "Source is mono" : "Stereo width is narrow" });
    if (analysis.dynamicRangeDb < 6.0f)
        issues.push_back({ "low_dynamic_range", clip01((6.0f - analysis.dynamicRangeDb) / 6.0f), "dynamics", "Crest factor is tight" });
    return issues;
}

void addClippingIssue(std::vector<models::Issue>& issues, float peak)
{
    if (peak >= 0.99f)
        issues.insert(issues.begin(), { "clipping", clip01(0.7f + (peak - 0.99f) * 10.0f), "full_band", "Peak amplitude is at or above 0.99 FS" });
}

void buildObjects(models::TrackDna& dna, const std::vector<models::Issue>& issues)
{
    models::SonoraObject map;
    map.type = "ARRANGEMENT_MAP";
    map.input = "structure detection";
    map.status = "generated";
    dna.objects.push_back(map);

    bool muddy = false, conflict = false, clip = false, dyn = false;
    for (const auto& issue : issues)
    {
        muddy = muddy || issue.type == "muddy_low_end";
        conflict = conflict || issue.type == "frequency_conflict";
        clip = clip || issue.type == "clipping";
        dyn = dyn || issue.type == "low_dynamic_range";
    }
    if (dna.lowEnd.risk == "high" || muddy || conflict)
    {
        models::SonoraObject eq;
        eq.type = "EQ_PROFILE";
        eq.input = dna.lowEnd.why;
        eq.status = "generated";
        eq.frequency = conflict ? 250.0f : 120.0f;
        if (dna.lowEnd.why.rfind("Sub", 0) == 0)
            eq.frequency = 55.0f;
        eq.gain = -3.0f;
        eq.q = 1.2f;
        dna.objects.push_back(eq);
    }
    if (clip || dyn)
    {
        models::SonoraObject master;
        master.type = "MASTER_CHAIN";
        master.input = "peak / crest";
        master.status = "generated";
        dna.objects.push_back(master);
    }
}

std::pair<std::string, bool> rootMode(const models::AudioAnalysis& analysis)
{
    std::string key = "A minor";
    if (analysis.key.key.has_value())
        key = *analysis.key.key;
    else if (!analysis.dna.keyName.empty())
        key = analysis.dna.keyName;
    const bool minor = key.find("minor") != std::string::npos || (!key.empty() && key.back() == 'm');
    auto token = key.substr(0, key.find(' '));
    if (token.size() >= 2 && (token[1] == '#' || token[1] == 'b'))
        token = token.substr(0, 2);
    else if (!token.empty())
        token = token.substr(0, 1);
    if (token.empty())
        token = "A";
    if (token[0] >= 'a' && token[0] <= 'z')
        token[0] = (char) (token[0] - 32);
    return { token, minor };
}

std::string fifthOf(const std::string& root)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int idx = 0;
    for (int i = 0; i < 12; ++i)
        if (root == names[i] || (root == "Db" && i == 1) || (root == "Eb" && i == 3) || (root == "Gb" && i == 6)
            || (root == "Ab" && i == 8) || (root == "Bb" && i == 10))
            idx = i;
    return names[(idx + 7) % 12];
}
} // namespace

bool loadFile(const juce::File& file, juce::AudioBuffer<float>& buffer, double& sampleRate, juce::String& error)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if (reader == nullptr)
    {
        error = "cannot decode file";
        return false;
    }
    const int n = (int) juce::jmin((juce::int64) reader->lengthInSamples, (juce::int64) (reader->sampleRate * kMaxSeconds));
    if (n <= 0)
    {
        error = "file is empty";
        return false;
    }
    buffer.setSize((int) juce::jmax((juce::uint32) 1, reader->numChannels), n, false, true, true);
    if (!reader->read(&buffer, 0, n, 0, true, true))
    {
        error = "cannot read samples";
        return false;
    }
    sampleRate = reader->sampleRate;
    return true;
}

Result analyze(const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    Result result;
    if (buffer.getNumSamples() <= 0 || sampleRate <= 0.0)
    {
        result.error = "empty buffer";
        return result;
    }

    const int maxSamples = (int) (kAnalyzeSr * kMaxSeconds);
    auto work = downsample(buffer, sampleRate, kAnalyzeSr, maxSamples);
    if (work.getNumSamples() <= 0)
        work = buffer;
    const double sr = work.getNumSamples() == buffer.getNumSamples() ? sampleRate : kAnalyzeSr;

    std::vector<float> mono;
    toMono(work, mono);
    const auto spec = meanSpectrum(mono, sr);
    const auto tl = computeTimeline(work, mono, sr);
    const float peak = peakAmp(buffer);
    const float rms = rmsAmp(mono);
    const float dyn = 20.0f * std::log10((peak + 1e-12f) / (rms + 1e-12f));

    auto& a = result.analysis;
    a.analyzerVersion = kAnalyzerVersion;
    a.durationSec = (float) (buffer.getNumSamples() / sampleRate);
    a.sampleRate = (int) std::lround(sampleRate);
    a.channels = buffer.getNumChannels();
    a.loudnessLufsApprox = std::round(loudnessApprox(work, sr) * 1000.0f) / 1000.0f;
    a.dynamicRangeDb = std::round(dyn * 1000.0f) / 1000.0f;
    a.stereoWidth = std::round(stereoWidth(work) * 10000.0f) / 10000.0f;
    a.bands = distribution(spec);
    estimateKey(spec, a.key);
    a.bpm = estimateBpm(tl.transient.empty() ? tl.flux : tl.transient, tl.hopSec);

    result.issues = detectIssues(a);
    addClippingIssue(result.issues, peak);

    auto& dna = a.dna;
    a.hasDna = true;
    dna.tempo = a.bpm;
    dna.keyName = a.key.key.value_or("");
    dna.keyConfidence = a.key.confidence;
    float transientMean = 0.0f;
    if (!tl.transient.empty())
        transientMean = std::accumulate(tl.transient.begin(), tl.transient.end(), 0.0f) / (float) tl.transient.size();
    dna.genreProfile = inferGenre(a.bpm, a.durationSec, a.bands, transientMean);
    dna.energyPeaks = peakEnvelope(mono);
    dna.energyCurve = downsampleCurve(tl.rms);
    if (!dna.energyCurve.empty())
    {
        dna.energyMean = std::accumulate(dna.energyCurve.begin(), dna.energyCurve.end(), 0.0f) / (float) dna.energyCurve.size();
        dna.energyPeak = *std::max_element(dna.energyCurve.begin(), dna.energyCurve.end());
        dna.energyMean = std::round(dna.energyMean * 10000.0f) / 10000.0f;
        dna.energyPeak = std::round(dna.energyPeak * 10000.0f) / 10000.0f;
    }
    dna.sections = detectStructure(tl, a.durationSec, a.bpm);
    buildMix(dna, spec, a);
    buildTranslation(dna, a);
    buildMasking(dna, spec);
    buildObjects(dna, result.issues);
    return result;
}

models::Harmony makeHarmony(const models::AudioAnalysis& analysis)
{
    models::Harmony h;
    h.key = analysis.key.key.value_or(analysis.dna.keyName.empty() ? "A minor" : analysis.dna.keyName);
    h.bars = 8;
    const bool minor = h.key.find("minor") != std::string::npos;
    h.chords = minor ? std::vector<std::string> { "Am9", "Fmaj7", "Cmaj7", "Esus4" }
                     : std::vector<std::string> { "Cmaj7", "Am7", "Fmaj7", "Gsus4" };
    return h;
}

models::MidiClip makeBass(const models::AudioAnalysis& analysis)
{
    const auto [root, minor] = rootMode(analysis);
    const auto fifth = fifthOf(root);
    models::MidiClip clip;
    clip.role = "bass";
    clip.key = analysis.key.key.value_or(analysis.dna.keyName.empty() ? "A minor" : analysis.dna.keyName);
    clip.bars = 8;
    const std::string q = minor ? "m" : "";
    clip.chords = { root + q, fifth + q, root + q, fifth + q };
    clip.notes = { root + "1", root + "2", fifth + "1", fifth + "2" };
    clip.pattern = { "x---", "x-x-", "x---", "x-x-", "x---", "--x-", "x---", "x-x-" };
    return clip;
}

models::MidiClip makePad(const models::AudioAnalysis& analysis)
{
    const auto [root, minor] = rootMode(analysis);
    const auto fifth = fifthOf(root);
    models::MidiClip clip;
    clip.role = "pad";
    clip.key = analysis.key.key.value_or(analysis.dna.keyName.empty() ? "A minor" : analysis.dna.keyName);
    clip.bars = 8;
    const std::string q = minor ? "m9" : "maj7";
    clip.chords = { root + q, fifth + q };
    clip.notes = { root + "3", fifth + "3", root + "4" };
    clip.pattern = { "xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx" };
    return clip;
}

models::DropPlan makeDrop(const models::AudioAnalysis& analysis)
{
    models::DropPlan plan;
    plan.sectionName = "body";
    plan.end = analysis.durationSec;
    plan.frequency = 3000.0f;
    plan.gain = 2.5f;
    plan.q = 1.1f;
    plan.energyTarget = 0.85f;
    if (analysis.hasDna)
    {
        const models::StructureSection* drop = nullptr;
        float best = -1.0f;
        for (const auto& s : analysis.dna.sections)
        {
            if (s.name == "drop")
            {
                drop = &s;
                break;
            }
            if (s.energy > best)
            {
                best = s.energy;
                drop = &s;
            }
        }
        if (drop != nullptr)
        {
            plan.sectionName = drop->name;
            plan.start = drop->start;
            plan.end = drop->end;
        }
    }
    plan.actions = {
        "Hold the first hit, then add the extra layer.",
        "Lift presence around 3 kHz so the drop reads on small speakers.",
        "Tighten ~100 Hz so the kick punches through the bass.",
    };
    return plan;
}

models::AssistAdvice makeAssist(const models::AudioAnalysis& analysis, const std::vector<models::Issue>& issues)
{
    models::AssistAdvice advice;
    advice.provider = "local";
    bool conflict = false, muddy = false, clip = false, hasDrop = false;
    for (const auto& issue : issues)
    {
        conflict = conflict || issue.type == "frequency_conflict";
        muddy = muddy || issue.type == "muddy_low_end";
        clip = clip || issue.type == "clipping";
    }
    if (analysis.hasDna)
        for (const auto& s : analysis.dna.sections)
            if (s.name == "drop")
                hasDrop = true;

    if (conflict)
    {
        advice.headline = "Vocals need space.";
        advice.detail = "Bass and mid information are fighting. Carve a pocket, then decide if the drop still needs weight.";
    }
    else if (muddy || (analysis.hasDna && analysis.dna.lowEnd.risk == "high"))
    {
        advice.headline = "Bass is crowding the mix.";
        advice.detail = analysis.hasDna && !analysis.dna.lowEnd.why.empty()
                            ? analysis.dna.lowEnd.why
                            : "Low end is taking the centre. Tighten it before you add more.";
    }
    else if (clip)
    {
        advice.headline = "Peaks are hitting the ceiling.";
        advice.detail = "Drop the input a few dB before the arrangement can open.";
    }
    else if (hasDrop)
    {
        advice.headline = "The drop can hit harder.";
        advice.detail = "Structure is there. Strengthen the landing, then write the bass that carries it.";
    }
    else
    {
        advice.headline = "The track has a solid foundation.";
        advice.detail = "No urgent collision. Create the next object when you want to move.";
    }
    advice.options = {
        { "strengthen_drop", "Strengthen drop" },
        { "create_bass", "Create bass" },
        { "fix_vocal_space", "Fix vocal space" },
        { "compare_reference", "Compare reference" },
    };
    return advice;
}

models::ReferenceReport compare(
    const models::AudioAnalysis& target,
    const models::AudioAnalysis& reference,
    const juce::String& targetName,
    const juce::String& referenceName)
{
    models::ReferenceReport report;
    report.targetFilename = targetName.toStdString();
    report.referenceFilename = referenceName.toStdString();
    const float tLow = target.bands.sub + target.bands.low;
    const float rLow = reference.bands.sub + reference.bands.low;
    const float tBright = target.bands.high + target.bands.air;
    const float rBright = reference.bands.high + reference.bands.air;
    report.gap.loudnessLufs = std::round((target.loudnessLufsApprox - reference.loudnessLufsApprox) * 100.0f) / 100.0f;
    report.gap.lowEnd = std::round((tLow - rLow) * 1000.0f) / 1000.0f;
    report.gap.stereo = std::round((target.stereoWidth - reference.stereoWidth) * 1000.0f) / 1000.0f;
    report.gap.brightness = std::round((tBright - rBright) * 1000.0f) / 1000.0f;

    auto note = [](float value, const juce::String& label, bool lufs) -> std::string {
        const float thresh = lufs ? 0.4f : 0.03f;
        if (std::abs(value) < thresh)
            return {};
        const juce::String word = value > 0.0f ? "hotter than" : "quieter than";
        if (lufs)
            return (label + " is " + juce::String(std::abs(value), 1) + " LUFS " + word + " the reference").toStdString();
        return (label + " is " + juce::String(juce::roundToInt(std::abs(value) * 100.0f)) + "% " + word + " the reference")
            .toStdString();
    };
    for (auto&& line : { note(report.gap.loudnessLufs, "Loudness", true),
                         note(report.gap.lowEnd, "Low end", false),
                         note(report.gap.stereo, "Stereo", false),
                         note(report.gap.brightness, "Brightness", false) })
        if (!line.empty())
            report.notes.push_back(std::move(line));
    if (report.notes.empty())
        report.notes.emplace_back("The two files sit close. No urgent target gap.");
    return report;
}

} // namespace sonora::engine
