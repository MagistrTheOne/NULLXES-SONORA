from __future__ import annotations

import librosa
import numpy as np

from app.audio.loader import to_mono
from app.schemas.dna import MaskingMap

# Frequency-role proxies. This is not stem separation.
ROLE_BANDS: dict[str, tuple[float, float]] = {
    "kick": (40.0, 90.0),
    "bass": (70.0, 200.0),
    "vocal": (300.0, 3500.0),
    "lead": (2000.0, 8000.0),
}
OVERLAP_BANDS: dict[tuple[str, str], tuple[float, float]] = {
    ("kick", "bass"): (70.0, 90.0),
    ("kick", "vocal"): (300.0, 400.0),
    ("kick", "lead"): (2000.0, 2500.0),
    ("bass", "vocal"): (250.0, 400.0),
    ("bass", "lead"): (2000.0, 2500.0),
    ("vocal", "lead"): (2000.0, 3500.0),
}


def _energy(spectrum: np.ndarray, freqs: np.ndarray, low_hz: float, high_hz: float) -> float:
    nyquist = float(freqs[-1]) if len(freqs) else 0.0
    mask = (freqs >= low_hz) & (freqs < min(high_hz, nyquist))
    return float(np.sum(spectrum[mask] ** 2))


def build_masking(samples: np.ndarray, sample_rate: int) -> MaskingMap:
    mono = to_mono(samples)
    magnitude = np.abs(librosa.stft(mono, n_fft=2048, hop_length=512))
    spectrum = magnitude.mean(axis=1)
    freqs = librosa.fft_frequencies(sr=sample_rate, n_fft=2048)
    roles = list(ROLE_BANDS.keys())
    role_energy = {name: _energy(spectrum, freqs, *ROLE_BANDS[name]) for name in roles}
    total = float(sum(role_energy.values()) + 1e-12)

    matrix: list[list[float]] = []
    for row in roles:
        line: list[float] = []
        for col in roles:
            if row == col:
                line.append(0.0)
                continue
            pair = (row, col) if (row, col) in OVERLAP_BANDS else (col, row)
            overlap = _energy(spectrum, freqs, *OVERLAP_BANDS[pair])
            pair_energy = role_energy[row] + role_energy[col] + 1e-12
            # Shared-band concentration, scaled by how present both roles are.
            presence = min(role_energy[row], role_energy[col]) / total
            value = (2.0 * overlap / pair_energy) * (0.35 + 2.4 * presence)
            line.append(round(float(np.clip(value, 0.0, 1.0)), 3))
        matrix.append(line)

    return MaskingMap(roles=roles, matrix=matrix)
