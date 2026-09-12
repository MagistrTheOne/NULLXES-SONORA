from __future__ import annotations

from app.schemas.audio import FrequencyDistribution


def infer_genre_profile(
    *,
    bpm: float,
    duration_sec: float,
    dist: FrequencyDistribution,
    transient_mean: float,
) -> list[str]:
    """Tempo + spectrum tags. Not a classifier and not a genre fact."""
    if duration_sec < 15.0 or bpm < 70.0:
        return []

    tags: list[str] = []
    low_end = dist.sub + dist.low
    if 118.0 <= bpm <= 132.0:
        tags.append("house")
        if low_end >= 0.42 and dist.high < 0.22:
            tags.append("deep house")
        if transient_mean >= 0.42 and dist.high >= 0.12:
            tags.append("slap house")
        if dist.mid >= 0.34 and 124.0 <= bpm <= 130.0:
            tags.append("tech house")
    elif 132.0 < bpm <= 148.0 and low_end >= 0.36:
        tags.append("techno")
    elif 168.0 <= bpm <= 180.0:
        tags.append("drum and bass")
    elif 80.0 <= bpm <= 100.0 and low_end >= 0.40:
        tags.append("hip hop")
    elif bpm > 0:
        tags.append("electronic")

    unique: list[str] = []
    for tag in tags:
        if tag not in unique:
            unique.append(tag)
    return unique[:3]
