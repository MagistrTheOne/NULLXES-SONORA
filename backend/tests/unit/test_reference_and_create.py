from __future__ import annotations

from app.audio.dsp.timeline import peak_envelope
from app.audio.intelligence.reference import compare_features
from app.schemas.audio import AudioFeatures
from app.services.dsp_advice import (
    heuristic_assist,
    heuristic_bass,
    heuristic_drop,
    heuristic_pad,
)
import numpy as np


def _features(**overrides) -> AudioFeatures:
    payload = {
        "analyzer_version": "SONORA_DSP_v0.3",
        "bpm": 124.0,
        "duration_sec": 32.0,
        "sample_rate": 22050,
        "channels": 2,
        "rms": 0.1,
        "peak": 0.4,
        "loudness_lufs_approx": -10.0,
        "dynamic_range_db": 12.0,
        "dynamic_range_label": "medium",
        "spectrum": {
            "centroid_hz": 1200.0,
            "rolloff_hz": 6000.0,
            "band_energies": {"low": 0.4, "mid": 0.4, "high": 0.2},
        },
        "frequency_distribution": {
            "sub": 0.18,
            "low": 0.22,
            "mid": 0.30,
            "high": 0.20,
            "air": 0.10,
        },
        "stereo_width": 0.45,
        "key_estimation": {"key": "A minor", "confidence": 0.7, "method": "chroma_cqt"},
    }
    payload.update(overrides)
    return AudioFeatures.model_validate(payload)


def test_peak_envelope_holds_transient() -> None:
    frames = np.zeros(4096, dtype=np.float32)
    frames[80] = 1.0
    peaks = peak_envelope(frames, bins=512)
    assert len(peaks) == 512
    assert max(peaks) == 1.0
    assert min(peaks) >= 0.0


def test_compare_features_reports_louder_darker_target() -> None:
    target = _features(
        loudness_lufs_approx=-8.0,
        stereo_width=0.2,
        frequency_distribution={
            "sub": 0.30,
            "low": 0.30,
            "mid": 0.25,
            "high": 0.10,
            "air": 0.05,
        },
    )
    reference = _features(
        loudness_lufs_approx=-12.0,
        stereo_width=0.6,
        frequency_distribution={
            "sub": 0.10,
            "low": 0.15,
            "mid": 0.30,
            "high": 0.30,
            "air": 0.15,
        },
    )
    report = compare_features(
        target,
        reference,
        target_filename="mix.wav",
        reference_filename="ref.wav",
    )
    assert report.gap.loudness_lufs == 4.0
    assert report.gap.low_end > 0
    assert report.gap.stereo < 0
    assert report.gap.brightness < 0
    assert report.object["type"] == "REFERENCE_REPORT"
    assert any("Loudness" in note for note in report.notes)
    assert any("Low end" in note for note in report.notes)


def test_heuristic_bass_and_pad_follow_key() -> None:
    features = {
        "key_estimation": {"key": "F# minor", "confidence": 0.8},
        "dna": {
            "structure": {
                "sections": [
                    {"name": "intro", "start": 0.0, "end": 8.0, "energy": 0.2},
                    {"name": "drop", "start": 8.0, "end": 16.0, "energy": 0.9},
                ]
            }
        },
        "duration_sec": 24.0,
    }
    bass = heuristic_bass(features)
    pad = heuristic_pad(features)
    drop = heuristic_drop(features)
    assert bass.role == "bass"
    assert bass.key == "F# minor"
    assert bass.notes[0].startswith("F#")
    assert "x---" in bass.pattern
    assert pad.role == "pad"
    assert pad.notes[0].endswith("3")
    assert drop.section_name == "drop"
    assert drop.start == 8.0
    assert drop.end == 16.0
    assert drop.frequency == 3000.0
    assert drop.actions


def test_heuristic_assist_uses_issues() -> None:
    advice = heuristic_assist(
        {
            "dna": {
                "structure": {"sections": [{"name": "drop", "start": 8, "end": 16, "energy": 0.8}]},
                "mix_character": {"low_end": {"risk": "high", "why": "Sub is taking the kick."}},
            }
        },
        [{"type": "frequency_conflict", "detail": "vocal / bass"}],
    )
    ids = {option.id for option in advice.options}
    assert advice.provider == "off"
    assert "space" in advice.headline.lower() or "vocal" in advice.headline.lower()
    assert "strengthen_drop" in ids
    assert "create_bass" in ids
    assert "fix_vocal_space" in ids
    assert "compare_reference" in ids
