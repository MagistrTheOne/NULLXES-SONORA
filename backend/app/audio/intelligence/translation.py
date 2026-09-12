from __future__ import annotations

from app.schemas.audio import FrequencyDistribution
from app.schemas.dna import MixCharacter, TranslationLab, TranslationTarget

# Remaining audibility weights. Not a headphone simulator — a band model.
PLAYBACK_WEIGHTS: dict[str, dict[str, float]] = {
    "phone": {"sub": 0.05, "low": 0.40, "mid": 1.00, "high": 0.90, "air": 0.45},
    "car": {"sub": 0.70, "low": 1.10, "mid": 0.90, "high": 0.75, "air": 0.30},
    "club": {"sub": 1.20, "low": 1.10, "mid": 0.85, "high": 0.70, "air": 0.25},
    "headphones": {"sub": 0.55, "low": 0.90, "mid": 1.00, "high": 1.00, "air": 0.90},
}


def _clip01(value: float) -> float:
    return max(0.0, min(1.0, value))


def _score(dist: FrequencyDistribution, weights: dict[str, float]) -> tuple[float, float, float]:
    shares = {
        "sub": dist.sub,
        "low": dist.low,
        "mid": dist.mid,
        "high": dist.high,
        "air": dist.air,
    }
    lost = 0.0
    boom = 0.0
    for band, share in shares.items():
        weight = weights[band]
        if weight < 1.0:
            lost += share * (1.0 - weight)
        else:
            boom += share * (weight - 1.0)
    score = _clip01(1.0 - lost * 1.20 - boom * 0.35)
    return score, lost, boom


def build_translation(dist: FrequencyDistribution, mix: MixCharacter) -> TranslationLab:
    punch_weak = (mix.low_end.sub or 0.0) >= 0.55 and (mix.low_end.control or 1.0) < 0.45
    sub_heavy = dist.sub >= 0.12
    targets: list[TranslationTarget] = []

    for name, weights in PLAYBACK_WEIGHTS.items():
        score, lost, boom = _score(dist, weights)
        issue = None
        reason = None
        action = None

        if name == "phone" and (lost >= 0.22 or (sub_heavy and punch_weak)):
            score = min(score, 0.48 if sub_heavy else score)
            issue = "Low end disappears on phone"
            reason = "Sub energy dominates below 60Hz"
            action = "Move bass information to 100-150Hz"
        elif name == "car" and boom >= 0.08 and dist.low >= 0.28:
            issue = "Low end blooms in a car"
            reason = "Cabin gain emphasizes 60-120Hz"
            action = "Tighten 80-120Hz before the sub"
        elif name == "club" and dist.sub >= 0.18 and dist.low < 0.16:
            issue = "Club system will feel hollow"
            reason = "Sub is present but punch band is thin"
            action = "Add harmonic bass information at 100-150Hz"
        elif name == "headphones" and (dist.high + dist.air) < 0.10:
            issue = "Headphones expose a dull top"
            reason = "High/air energy share is low"
            action = "Open 6-10kHz after the mix body is stable"
        elif name == "headphones" and mix.stereo.risk == "high":
            issue = "Headphones collapse the image"
            reason = mix.stereo.why
            action = "Restore mid/side width above 400Hz"

        targets.append(
            TranslationTarget(
                name=name,  # type: ignore[arg-type]
                score=round(score, 3),
                issue=issue,
                reason=reason,
                action=action,
            )
        )

    return TranslationLab(targets=targets)
