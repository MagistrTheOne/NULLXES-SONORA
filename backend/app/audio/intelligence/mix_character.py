from __future__ import annotations

import librosa
import numpy as np

from app.audio.loader import to_mono
from app.schemas.audio import AudioFeatures
from app.schemas.dna import MixAxis, MixCharacter

FINE_EDGES: dict[str, tuple[float, float]] = {
    "sub": (20.0, 60.0),
    "mud": (80.0, 120.0),
    "punch": (100.0, 150.0),
    "low": (60.0, 250.0),
    "mid": (250.0, 2000.0),
    "high": (2000.0, 8000.0),
    "air": (8000.0, 20000.0),
}


def _clip01(value: float) -> float:
    return float(np.clip(value, 0.0, 1.0))


def fine_band_shares(samples: np.ndarray, sample_rate: int) -> dict[str, float]:
    mono = to_mono(samples)
    magnitude = np.abs(librosa.stft(mono, n_fft=2048, hop_length=512))
    spectrum = magnitude.mean(axis=1)
    freqs = librosa.fft_frequencies(sr=sample_rate, n_fft=2048)
    energies: dict[str, float] = {}
    for name, (low_hz, high_hz) in FINE_EDGES.items():
        nyquist = float(freqs[-1]) if len(freqs) else 0.0
        mask = (freqs >= low_hz) & (freqs < min(high_hz, nyquist))
        energies[name] = float(np.sum(spectrum[mask] ** 2))
    total = float(sum(energies.values()) + 1e-12)
    return {name: value / total for name, value in energies.items()}


def _risk(level: float) -> str:
    if level >= 0.62:
        return "high"
    if level >= 0.38:
        return "medium"
    return "low"


def build_mix_character(samples: np.ndarray, sample_rate: int, features: AudioFeatures) -> MixCharacter:
    fine = fine_band_shares(samples, sample_rate)
    dist = features.frequency_distribution
    low_end = dist.sub + dist.low
    mud_ratio = fine["mud"] / (fine["sub"] + fine["low"] + 1e-12)
    punch = fine["punch"]
    sub_score = _clip01(dist.sub / 0.18)
    low_score = _clip01(dist.low / 0.28)
    control = _clip01(1.0 - mud_ratio * 1.45)

    if mud_ratio >= 0.28:
        why = "Excess energy 80-120Hz"
        risk_level = max(low_end, mud_ratio + 0.2)
    elif dist.sub >= 0.14 and punch < 0.06:
        why = "Sub energy dominates below 60Hz"
        risk_level = max(low_end, dist.sub + 0.25)
    elif low_end >= 0.50:
        why = "Low-end share is high relative to the rest of the spectrum"
        risk_level = low_end
    else:
        why = "Low end is balanced"
        risk_level = low_end * 0.7

    brightness = _clip01((dist.high + dist.air) / 0.28)
    if brightness < 0.28:
        bright_why = "High-frequency energy is thin"
        bright_risk = "medium" if brightness < 0.18 else "low"
    elif brightness > 0.85:
        bright_why = "Top end is aggressive relative to body"
        bright_risk = "medium"
    else:
        bright_why = "Brightness is in range"
        bright_risk = "low"

    if features.channels == 1:
        stereo_why = "Source is mono"
        stereo_risk = "high"
        stereo_value = 0.0
    elif features.stereo_width < 0.08:
        stereo_why = f"Stereo width is {features.stereo_width:.3f}"
        stereo_risk = "high"
        stereo_value = features.stereo_width
    elif features.stereo_width > 0.62:
        stereo_why = "Side energy is unusually high"
        stereo_risk = "medium"
        stereo_value = features.stereo_width
    else:
        stereo_why = "Stereo image is usable"
        stereo_risk = "low"
        stereo_value = features.stereo_width

    if features.dynamic_range_db < 6.0:
        dyn_why = f"Crest factor is {features.dynamic_range_db:.2f} dB"
        dyn_risk = "high"
    elif features.dynamic_range_db < 8.0:
        dyn_why = f"Crest factor is {features.dynamic_range_db:.2f} dB"
        dyn_risk = "medium"
    else:
        dyn_why = "Dynamic range is usable"
        dyn_risk = "low"

    return MixCharacter(
        low_end=MixAxis(
            sub=round(sub_score, 3),
            low=round(low_score, 3),
            control=round(control, 3),
            risk=_risk(risk_level),
            why=why,
            method="band_share + 80_120_concentration",
        ),
        brightness=MixAxis(
            value=round(brightness, 3),
            risk=bright_risk,
            why=bright_why,
            method="high+air share",
        ),
        stereo=MixAxis(
            value=round(stereo_value, 3),
            risk=stereo_risk,
            why=stereo_why,
            method="side/(mid+side)",
        ),
        dynamics=MixAxis(
            value=round(_clip01(features.dynamic_range_db / 16.0), 3),
            risk=dyn_risk,
            why=dyn_why,
            method="crest_factor_db",
        ),
    )
