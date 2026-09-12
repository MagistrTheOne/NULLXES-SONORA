from __future__ import annotations

import librosa
import numpy as np

from app.audio.loader import to_mono

BAND_EDGES: dict[str, tuple[float, float]] = {
    "sub": (20.0, 60.0),
    "low": (60.0, 250.0),
    "mid": (250.0, 2000.0),
    "high": (2000.0, 8000.0),
    "air": (8000.0, 20000.0),
}


def _band_mask(freqs: np.ndarray, low_hz: float, high_hz: float) -> np.ndarray:
    nyquist = float(freqs[-1]) if len(freqs) else 0.0
    upper = min(high_hz, nyquist)
    return (freqs >= low_hz) & (freqs < upper)


def frequency_distribution(samples: np.ndarray, sample_rate: int) -> dict[str, float]:
    mono = to_mono(samples)
    magnitude = np.abs(librosa.stft(mono, n_fft=2048, hop_length=512))
    spectrum = magnitude.mean(axis=1)
    freqs = librosa.fft_frequencies(sr=sample_rate, n_fft=2048)
    energies: dict[str, float] = {}
    for name, (low_hz, high_hz) in BAND_EDGES.items():
        mask = _band_mask(freqs, low_hz, high_hz)
        energies[name] = float(np.sum(spectrum[mask] ** 2))
    total = float(sum(energies.values()) + 1e-12)
    return {name: value / total for name, value in energies.items()}


def spectrum_summary(samples: np.ndarray, sample_rate: int) -> dict:
    mono = to_mono(samples)
    centroid = float(librosa.feature.spectral_centroid(y=mono, sr=sample_rate).mean())
    rolloff = float(librosa.feature.spectral_rolloff(y=mono, sr=sample_rate).mean())
    return {
        "centroid_hz": centroid,
        "rolloff_hz": rolloff,
        "band_energies": frequency_distribution(samples, sample_rate),
    }
