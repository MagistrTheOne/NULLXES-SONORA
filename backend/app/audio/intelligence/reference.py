from __future__ import annotations

from app.schemas.audio import AudioFeatures
from app.schemas.reference import ReferenceGap, ReferenceReport


def _clip_note(value: float, label: str, unit: str, invert_good: bool = False) -> str | None:
    if abs(value) < (0.4 if unit == "LUFS" else 0.03):
        return None
    hotter = value > 0
    if invert_good:
        hotter = not hotter
    word = "hotter than" if hotter else "quieter than"
    if unit == "%":
        return f"{label} is {abs(value) * 100:.0f}% {word} the reference"
    return f"{label} is {abs(value):.1f} {unit} {word} the reference"


def compare_features(
    target: AudioFeatures,
    reference: AudioFeatures,
    *,
    target_filename: str,
    reference_filename: str,
) -> ReferenceReport:
    t_low = target.frequency_distribution.sub + target.frequency_distribution.low
    r_low = reference.frequency_distribution.sub + reference.frequency_distribution.low
    t_bright = target.frequency_distribution.high + target.frequency_distribution.air
    r_bright = reference.frequency_distribution.high + reference.frequency_distribution.air
    gap = ReferenceGap(
        loudness_lufs=round(target.loudness_lufs_approx - reference.loudness_lufs_approx, 2),
        low_end=round(t_low - r_low, 3),
        stereo=round(target.stereo_width - reference.stereo_width, 3),
        brightness=round(t_bright - r_bright, 3),
    )
    notes = [
        note
        for note in (
            _clip_note(gap.loudness_lufs, "Loudness", "LUFS"),
            _clip_note(gap.low_end, "Low end", "%"),
            _clip_note(gap.stereo, "Stereo", "%"),
            _clip_note(gap.brightness, "Brightness", "%"),
        )
        if note
    ]
    if not notes:
        notes = ["The two files sit close. No urgent target gap."]
    report_object = {
        "type": "REFERENCE_REPORT",
        "input": reference_filename,
        "parameters": gap.model_dump(),
        "status": "generated",
    }
    return ReferenceReport(
        target_filename=target_filename,
        reference_filename=reference_filename,
        gap=gap,
        notes=notes,
        object=report_object,
    )
