from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import librosa
import numpy as np
import soundfile as sf

from app.core.exceptions import AnalysisError, UnsupportedMediaError
from app.services.storage import ALLOWED_EXTENSIONS


@dataclass(frozen=True)
class LoadedAudio:
    samples: np.ndarray
    sample_rate: int
    channels: int
    duration_sec: float


def _ensure_channel_first(samples: np.ndarray) -> np.ndarray:
    if samples.ndim == 1:
        return samples
    if samples.ndim != 2:
        raise AnalysisError("Unexpected audio shape")
    # soundfile: (frames, channels); librosa: (channels, frames)
    if samples.shape[0] < samples.shape[1]:
        return samples.T
    return samples


def load_audio(path: Path) -> LoadedAudio:
    if not path.exists():
        raise AnalysisError(f"Audio file not found: {path}")
    ext = path.suffix.lstrip(".").lower()
    if ext not in ALLOWED_EXTENSIONS:
        raise UnsupportedMediaError(f"Unsupported audio format: .{ext}")

    try:
        data, sample_rate = sf.read(str(path), always_2d=False)
        samples = np.asarray(data, dtype=np.float32)
        if samples.ndim == 2:
            samples = samples.T
    except Exception:
        samples, sample_rate = librosa.load(str(path), sr=None, mono=False)
        samples = np.asarray(samples, dtype=np.float32)

    samples = _ensure_channel_first(samples)
    if samples.ndim == 1:
        channels = 1
        n_frames = int(samples.shape[0])
    else:
        channels = int(samples.shape[0])
        n_frames = int(samples.shape[1])

    if n_frames == 0:
        raise AnalysisError("Audio file is empty")

    duration_sec = float(n_frames) / float(sample_rate)
    return LoadedAudio(
        samples=samples,
        sample_rate=int(sample_rate),
        channels=channels,
        duration_sec=duration_sec,
    )


def to_mono(samples: np.ndarray) -> np.ndarray:
    if samples.ndim == 1:
        return samples
    return np.mean(samples, axis=0)
