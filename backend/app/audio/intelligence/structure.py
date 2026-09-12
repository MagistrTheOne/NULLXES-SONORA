from __future__ import annotations

import numpy as np
from scipy.signal import find_peaks

from app.audio.dsp.timeline import Timeline, novelty_curve, smooth_energy
from app.schemas.dna import StructureSection, TrackStructure


def _section_mean(values: np.ndarray, start: int, end: int) -> float:
    slice_ = values[start:end]
    if slice_.size == 0:
        return 0.0
    return float(np.clip(np.mean(slice_), 0.0, 1.0))


def _boundaries(novelty: np.ndarray, hop_sec: float, duration: float, bpm: float) -> list[int]:
    n = int(novelty.size)
    if n <= 2:
        return [0, max(1, n)]

    beat = 60.0 / bpm if bpm >= 60.0 else 0.5
    if duration < 45.0:
        min_len = max(4.0, duration / 6.0)
    else:
        min_len = max(6.0, min(16.0, 8.0 * beat))
    min_frames = max(2, int(round(min_len / max(hop_sec, 1e-6))))

    peaks, _ = find_peaks(novelty, distance=min_frames, prominence=0.06)
    points = [0, *peaks.tolist(), n]
    merged = [points[0]]
    for point in points[1:]:
        if point - merged[-1] >= min_frames or point == n:
            merged.append(point)
    if merged[-1] != n:
        merged.append(n)
    if len(merged) == 2 and n > min_frames * 2:
        third = n // 3
        merged = [0, third, 2 * third, n]
    return merged


def _label(sections: list[dict]) -> None:
    if len(sections) == 1:
        sections[0]["name"] = "body"
        return

    energies = np.array([item["energy"] for item in sections], dtype=np.float64)
    drop_i = int(np.argmax(energies))
    median = float(np.median(energies))
    spread = float(np.max(energies) - np.min(energies))

    if spread < 0.12:
        sections[0]["name"] = "intro" if len(sections) > 2 else "body"
        if len(sections) == 2:
            sections[0]["name"] = "intro"
            sections[1]["name"] = "outro"
            return
        if len(sections) > 2:
            sections[-1]["name"] = "outro"
            for item in sections[1:-1]:
                item["name"] = "groove"
        return

    for index, item in enumerate(sections):
        energy = item["energy"]
        if index == 0 and energy <= median + 0.05:
            item["name"] = "intro"
        elif index == len(sections) - 1 and energy <= median + 0.08:
            item["name"] = "outro"
        elif index == drop_i and energy >= max(0.55, median + 0.08):
            item["name"] = "drop"
        elif index < drop_i and energies[drop_i] - energy >= 0.10:
            rising = index == 0 or energy >= sections[index - 1]["energy"] - 0.02
            item["name"] = "build" if rising or index + 1 == drop_i else "groove"
        elif index > drop_i and energies[drop_i] - energy >= 0.15:
            item["name"] = "break"
        else:
            item["name"] = "groove"

    if not any(item["name"] == "drop" for item in sections):
        sections[drop_i]["name"] = "drop"


def detect_structure(timeline: Timeline, duration_sec: float, bpm: float) -> TrackStructure:
    if timeline.times.size == 0 or duration_sec <= 0:
        return TrackStructure(
            sections=[
                StructureSection(
                    name="body",
                    start=0.0,
                    end=0.0,
                    energy=0.0,
                    bass_energy=0.0,
                    transient_density=0.0,
                    stereo_width=0.0,
                )
            ]
        )

    energy = smooth_energy(timeline)
    novelty = novelty_curve(timeline, energy)
    if duration_sec < 20.0:
        bounds = [0, int(energy.size)]
    else:
        bounds = _boundaries(novelty, timeline.hop_sec, duration_sec, bpm)

    raw: list[dict] = []
    for start, end in zip(bounds[:-1], bounds[1:], strict=True):
        end = max(end, start + 1)
        t0 = float(timeline.times[start])
        t1 = float(timeline.times[min(end, len(timeline.times)) - 1] + timeline.hop_sec)
        raw.append(
            {
                "name": "groove",
                "start": round(max(0.0, t0), 3),
                "end": round(min(duration_sec, t1), 3),
                "energy": round(_section_mean(energy, start, end), 4),
                "bass_energy": round(_section_mean(timeline.bass_share, start, end), 4),
                "transient_density": round(_section_mean(timeline.transient, start, end), 4),
                "stereo_width": round(_section_mean(timeline.width, start, end), 4),
            }
        )
    if raw:
        raw[-1]["end"] = round(duration_sec, 3)
        raw[0]["start"] = 0.0
    _label(raw)
    return TrackStructure(sections=[StructureSection.model_validate(item) for item in raw])
