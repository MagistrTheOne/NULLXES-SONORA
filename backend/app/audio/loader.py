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


def from_soundfile_layout(data: np.ndarray) -> np.ndarray:
    """soundfile: (frames,) or (frames, channels) -> mono or (channels, frames)."""
    samples = np.asarray(data, dtype=np.float32)
    if samples.ndim == 1:
        return samples
    if samples.ndim != 2:
        raise AnalysisError("Unexpected audio shape")
    return samples.T


def from_librosa_layout(data: np.ndarray) -> np.ndarray:
    """librosa: (n,) or (channels, frames). Already channel-first — do not guess."""
    samples = np.asarray(data, dtype=np.float32)
    if samples.ndim not in (1, 2):
        raise AnalysisError("Unexpected audio shape")
    return samples


def _describe(samples: np.ndarray) -> tuple[int, int]:
    if samples.ndim == 1:
        return 1, int(samples.shape[0])
    return int(samples.shape[0]), int(samples.shape[1])


def load_audio(path: Path) -> LoadedAudio:
    if not path.exists():
        raise AnalysisError(f"Audio file not found: {path}")
    ext = path.suffix.lstrip(".").lower()
    if ext not in ALLOWED_EXTENSIONS:
        raise UnsupportedMediaError(f"Unsupported audio format: .{ext}")

    try:
        data, sample_rate = sf.read(str(path), always_2d=True)
        samples = from_soundfile_layout(data)
    except Exception:
        data, sample_rate = librosa.load(str(path), sr=None, mono=False)
        samples = from_librosa_layout(data)

    channels, n_frames = _describe(samples)
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
