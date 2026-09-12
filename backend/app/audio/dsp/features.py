from __future__ import annotations

import librosa
import numpy as np

from app.audio.dsp.key import estimate_key
from app.audio.dsp.loudness import (
    dynamic_range_db,
    dynamic_range_label,
    loudness_lufs_approx,
    peak_amplitude,
    rms_amplitude,
)
from app.audio.dsp.spectrum import frequency_distribution, spectrum_summary
from app.audio.dsp.stereo import stereo_width
from app.audio.intelligence.dna import build_track_dna
from app.audio.intelligence.issues import detect_issues
from app.audio.loader import LoadedAudio, to_mono
from app.core.config import get_settings
from app.schemas.audio import AudioFeatures


def estimate_bpm(samples: np.ndarray, sample_rate: int) -> float:
    mono = to_mono(samples)
    tempo, _ = librosa.beat.beat_track(y=mono, sr=sample_rate, units="time")
    value = float(np.atleast_1d(tempo)[0])
    if not np.isfinite(value) or value <= 0:
        return 0.0
    return round(value, 2)


def extract_features(audio: LoadedAudio, analyzer_version: str | None = None) -> AudioFeatures:
    settings = get_settings()
    version = analyzer_version or settings.analyzer_version
    samples = audio.samples
    sr = audio.sample_rate

    peak = peak_amplitude(samples)
    rms = rms_amplitude(samples)
    dyn = dynamic_range_db(peak, rms)
    distribution = frequency_distribution(samples, sr)

    payload = {
        "analyzer_version": version,
        "bpm": estimate_bpm(samples, sr),
        "duration_sec": round(audio.duration_sec, 4),
        "sample_rate": sr,
        "channels": audio.channels,
        "rms": float(rms),
        "peak": float(peak),
        "loudness_lufs_approx": round(loudness_lufs_approx(samples, sr), 3),
        "dynamic_range_db": round(dyn, 3),
        "dynamic_range_label": dynamic_range_label(dyn),
        "spectrum": spectrum_summary(samples, sr),
        "frequency_distribution": distribution,
        "stereo_width": round(stereo_width(samples), 4),
        "key_estimation": estimate_key(samples, sr),
    }
    features = AudioFeatures.model_validate(payload)
    issues = detect_issues(features)
    return features.model_copy(update={"dna": build_track_dna(audio, features, issues)})
