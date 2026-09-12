from __future__ import annotations

from typing import Any


def compact_track(
    features: dict[str, Any] | None,
    issues: list[Any] | None = None,
) -> dict[str, Any]:
    """DNA + issues for the LLM. No STFT, no peak/RMS curves."""
    features = features or {}
    dna = features.get("dna") or {}
    energy = dna.get("energy") or {}
    mix = dna.get("mix_character") or {}
    structure = dna.get("structure") or {}
    translation = dna.get("translation") or {}
    return {
        "bpm": features.get("bpm"),
        "duration_sec": features.get("duration_sec"),
        "loudness_lufs_approx": features.get("loudness_lufs_approx"),
        "dynamic_range_db": features.get("dynamic_range_db"),
        "stereo_width": features.get("stereo_width"),
        "frequency_distribution": features.get("frequency_distribution"),
        "key_estimation": features.get("key_estimation"),
        "identity": dna.get("identity"),
        "energy": {"mean": energy.get("mean"), "peak": energy.get("peak")},
        "structure": [
            {
                "name": section.get("name"),
                "start": section.get("start"),
                "end": section.get("end"),
                "energy": section.get("energy"),
            }
            for section in structure.get("sections") or []
            if isinstance(section, dict)
        ],
        "mix_character": {
            axis: {"risk": value.get("risk"), "why": value.get("why")}
            for axis, value in mix.items()
            if isinstance(value, dict)
        },
        "translation": [
            {
                "name": target.get("name"),
                "score": target.get("score"),
                "issue": target.get("issue"),
                "action": target.get("action"),
            }
            for target in translation.get("targets") or []
            if isinstance(target, dict)
        ],
        "issues": issues or [],
    }
