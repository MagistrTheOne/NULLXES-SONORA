from __future__ import annotations

import librosa
import numpy as np

from app.audio.loader import to_mono

PITCH_CLASSES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

# Krumhansl-Kessler key profiles. Correlation is an estimate, not a fact.
MAJOR_PROFILE = np.array(
    [6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88],
    dtype=np.float64,
)
MINOR_PROFILE = np.array(
    [6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17],
    dtype=np.float64,
)


def _normalize(vector: np.ndarray) -> np.ndarray:
    norm = np.linalg.norm(vector)
    if norm < 1e-12:
        return vector
    return vector / norm


def estimate_key(samples: np.ndarray, sample_rate: int) -> dict:
    try:
        return _estimate_key(samples, sample_rate)
    except Exception:
        return {"key": None, "confidence": 0.0, "method": "chroma_cqt"}


def _estimate_key(samples: np.ndarray, sample_rate: int) -> dict:
    mono = to_mono(samples)
    chroma = librosa.feature.chroma_cqt(y=mono, sr=sample_rate)
    chroma_mean = _normalize(chroma.mean(axis=1).astype(np.float64))
    major = _normalize(MAJOR_PROFILE)
    minor = _normalize(MINOR_PROFILE)

    scores: list[tuple[float, str]] = []
    for shift in range(12):
        name = PITCH_CLASSES[shift]
        major_score = float(np.dot(chroma_mean, np.roll(major, shift)))
        minor_score = float(np.dot(chroma_mean, np.roll(minor, shift)))
        scores.append((major_score, f"{name} major"))
        scores.append((minor_score, f"{name} minor"))

    scores.sort(key=lambda item: item[0], reverse=True)
    best_score, best_key = scores[0]
    second_score = scores[1][0] if len(scores) > 1 else 0.0
    gap = max(0.0, best_score - second_score)
    confidence = float(np.clip(max(0.0, best_score) * (0.45 + gap), 0.0, 1.0))

    key: str | None = best_key if best_score > 0.35 and confidence >= 0.20 else None
    return {
        "key": key,
        "confidence": round(confidence, 4),
        "method": "chroma_cqt",
    }
