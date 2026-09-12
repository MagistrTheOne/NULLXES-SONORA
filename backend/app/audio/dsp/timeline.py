from __future__ import annotations

from dataclasses import dataclass

import librosa
import numpy as np

from app.audio.loader import to_mono

N_FFT = 2048
HOP_SEC = 0.25
BASS_HZ = (20.0, 250.0)


@dataclass(frozen=True)
class Timeline:
    hop_sec: float
    hop_length: int
    times: np.ndarray
    rms: np.ndarray
    flux: np.ndarray
    transient: np.ndarray
    bass_share: np.ndarray
    width: np.ndarray
    stft: np.ndarray
    freqs: np.ndarray


def _minmax(values: np.ndarray) -> np.ndarray:
    lo = float(np.min(values))
    hi = float(np.max(values))
    if hi - lo < 1e-9:
        return np.full_like(values, 0.5, dtype=np.float64)
    return (values - lo) / (hi - lo)


def _smooth(values: np.ndarray, window: int) -> np.ndarray:
    size = max(1, int(window))
    if size <= 1 or values.size == 0:
        return values.astype(np.float64, copy=False)
    kernel = np.ones(size, dtype=np.float64) / float(size)
    return np.convolve(values.astype(np.float64), kernel, mode="same")


def _width_envelope(samples: np.ndarray, n_frames: int, hop: int, frame_len: int) -> np.ndarray:
    if samples.ndim == 1 or samples.shape[0] < 2:
        return np.zeros(n_frames, dtype=np.float64)
    n = int(samples.shape[1])
    out = np.zeros(n_frames, dtype=np.float64)
    half = frame_len // 2
    for index in range(n_frames):
        center = index * hop
        start = max(0, center - half)
        end = min(n, start + frame_len)
        left = samples[0, start:end]
        right = samples[1, start:end]
        mid = 0.5 * (left + right)
        side = 0.5 * (left - right)
        energy_mid = float(np.mean(mid**2))
        energy_side = float(np.mean(side**2))
        out[index] = energy_side / (energy_mid + energy_side + 1e-12)
    return out


def compute_timeline(samples: np.ndarray, sample_rate: int, hop_sec: float = HOP_SEC) -> Timeline:
    mono = to_mono(samples)
    hop = max(1, int(round(sample_rate * hop_sec)))
    stft = np.abs(librosa.stft(mono, n_fft=N_FFT, hop_length=hop, center=True))
    freqs = librosa.fft_frequencies(sr=sample_rate, n_fft=N_FFT)
    n_frames = int(stft.shape[1])

    rms = librosa.feature.rms(y=mono, frame_length=N_FFT, hop_length=hop, center=True)[0]
    if rms.size != n_frames:
        rms = np.resize(rms, n_frames)

    delta = np.diff(stft, axis=1, prepend=stft[:, :1])
    flux = np.sqrt(np.sum(np.clip(delta, 0.0, None) ** 2, axis=0))

    onset = librosa.onset.onset_strength(y=mono, sr=sample_rate, hop_length=hop)
    if onset.size != n_frames:
        onset = np.resize(onset, n_frames)

    bass_mask = (freqs >= BASS_HZ[0]) & (freqs < BASS_HZ[1])
    bass = np.sum(stft[bass_mask] ** 2, axis=0)
    total = np.sum(stft**2, axis=0) + 1e-12
    bass_share = bass / total

    times = librosa.frames_to_time(np.arange(n_frames), sr=sample_rate, hop_length=hop)
    width = _width_envelope(samples, n_frames, hop, N_FFT)

    return Timeline(
        hop_sec=float(hop / sample_rate),
        hop_length=hop,
        times=times.astype(np.float64),
        rms=rms.astype(np.float64),
        flux=flux.astype(np.float64),
        transient=_minmax(onset.astype(np.float64)),
        bass_share=np.clip(bass_share, 0.0, 1.0).astype(np.float64),
        width=np.clip(width, 0.0, 1.0),
        stft=stft,
        freqs=freqs,
    )


def peak_envelope(samples: np.ndarray, bins: int = 512) -> list[float]:
    """Peak-hold waveform for UI. Not a DAW overview meter."""
    mono = to_mono(samples)
    if mono.size == 0:
        return [0.0] * bins
    if mono.size <= bins:
        peak = float(np.max(np.abs(mono)) + 1e-12)
        return [round(float(abs(sample) / peak), 4) for sample in mono]
    edges = np.linspace(0, mono.size, bins + 1).astype(int)
    out: list[float] = []
    peak = float(np.max(np.abs(mono)) + 1e-12)
    for start, end in zip(edges[:-1], edges[1:], strict=True):
        end = max(end, start + 1)
        out.append(round(float(np.max(np.abs(mono[start:end])) / peak), 4))
    return out


def downsample_curve(values: np.ndarray, bins: int = 48) -> list[float]:
    if values.size == 0:
        return [0.0] * bins
    if values.size <= bins:
        return [round(float(item), 4) for item in _minmax(values)]
    edges = np.linspace(0, values.size, bins + 1).astype(int)
    curve = []
    scaled = _minmax(values)
    for start, end in zip(edges[:-1], edges[1:], strict=True):
        end = max(end, start + 1)
        curve.append(round(float(np.mean(scaled[start:end])), 4))
    return curve


def smooth_energy(timeline: Timeline) -> np.ndarray:
    window = max(3, int(round(1.0 / max(timeline.hop_sec, 1e-6))))
    return _smooth(_minmax(timeline.rms), window)


def novelty_curve(timeline: Timeline, energy: np.ndarray) -> np.ndarray:
    flux = _minmax(timeline.flux)
    window = max(3, int(round(0.75 / max(timeline.hop_sec, 1e-6))))
    novelty = 0.65 * np.abs(np.gradient(energy)) + 0.35 * flux
    return _smooth(novelty, window)
