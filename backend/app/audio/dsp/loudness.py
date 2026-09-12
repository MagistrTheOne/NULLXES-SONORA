from __future__ import annotations

import numpy as np
from scipy.signal import lfilter, resample_poly

from app.audio.loader import to_mono

# ITU-R BS.1770-4 K-weighting coefficients at 48 kHz.
# This is an approximation: no gating, so the field is loudness_lufs_approx.
_K_SHELF_B = np.array([1.53512485958697, -2.69169618940638, 1.19839281085285])
_K_SHELF_A = np.array([1.0, -1.69065929318241, 0.73248077421585])
_K_HP_B = np.array([1.0, -2.0, 1.0])
_K_HP_A = np.array([1.0, -1.99004745483398, 0.99007225036621])
_TARGET_SR = 48000


def _resample_to_48k(channel: np.ndarray, sample_rate: int) -> np.ndarray:
    if sample_rate == _TARGET_SR:
        return channel.astype(np.float64, copy=False)
    from math import gcd

    g = gcd(sample_rate, _TARGET_SR)
    up = _TARGET_SR // g
    down = sample_rate // g
    return resample_poly(channel.astype(np.float64), up, down)


def _k_weight(channel: np.ndarray) -> np.ndarray:
    staged = lfilter(_K_SHELF_B, _K_SHELF_A, channel)
    return lfilter(_K_HP_B, _K_HP_A, staged)


def loudness_lufs_approx(samples: np.ndarray, sample_rate: int) -> float:
    """K-weighted mean-square loudness. Not a certified EBU R128 meter."""
    if samples.ndim == 1:
        channels = [samples]
    else:
        channels = [samples[i] for i in range(samples.shape[0])]

    weighted_ms = []
    channel_gains = [1.0, 1.0, 1.0, 1.41, 1.41]  # BS.1770 channel weights (L,R,C,Ls,Rs)
    for index, channel in enumerate(channels):
        resampled = _resample_to_48k(channel, sample_rate)
        filtered = _k_weight(resampled)
        gain = channel_gains[index] if index < len(channel_gains) else 1.0
        weighted_ms.append(gain * float(np.mean(filtered**2)))

    total = float(np.sum(weighted_ms))
    return float(-0.691 + 10.0 * np.log10(total + 1e-12))


def peak_amplitude(samples: np.ndarray) -> float:
    return float(np.max(np.abs(samples)))


def rms_amplitude(samples: np.ndarray) -> float:
    mono = to_mono(samples)
    return float(np.sqrt(np.mean(mono**2) + 1e-12))


def dynamic_range_db(peak: float, rms: float) -> float:
    return float(20.0 * np.log10((peak + 1e-12) / (rms + 1e-12)))


def dynamic_range_label(dynamic_range: float) -> str:
    if dynamic_range < 8.0:
        return "low"
    if dynamic_range < 14.0:
        return "medium"
    return "high"
