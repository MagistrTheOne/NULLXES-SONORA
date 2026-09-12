from __future__ import annotations

from typing import Any

from app.ai.schemas.harmony import HarmonyProgression
from app.ai.schemas.recommendation import LLMRecommendationItem, LLMRecommendationResult


def heuristic_recommendations(issues: list[Any] | None) -> LLMRecommendationResult:
    items: list[LLMRecommendationItem] = []
    for raw in issues or []:
        issue = raw if isinstance(raw, dict) else {}
        kind = str(issue.get("type") or "unknown")
        detail = str(issue.get("detail") or kind)
        if kind == "clipping":
            items.append(
                LLMRecommendationItem(
                    action="Reduce peak gain",
                    target=kind,
                    frequency_hz=None,
                    rationale=detail,
                    priority=1,
                )
            )
        elif kind == "muddy_low_end":
            items.append(
                LLMRecommendationItem(
                    action="Create EQ profile",
                    target=kind,
                    frequency_hz=120.0,
                    rationale=detail,
                    priority=2,
                )
            )
        elif kind == "frequency_conflict":
            items.append(
                LLMRecommendationItem(
                    action="Create EQ profile",
                    target=kind,
                    frequency_hz=250.0,
                    rationale=detail,
                    priority=3,
                )
            )
        elif kind == "narrow_stereo":
            items.append(
                LLMRecommendationItem(
                    action="Widen mid/side",
                    target=kind,
                    frequency_hz=None,
                    rationale=detail,
                    priority=4,
                )
            )
        elif kind == "low_dynamic_range":
            items.append(
                LLMRecommendationItem(
                    action="Restore dynamics",
                    target=kind,
                    frequency_hz=None,
                    rationale=detail,
                    priority=5,
                )
            )
        else:
            items.append(
                LLMRecommendationItem(
                    action="Create EQ profile",
                    target=kind,
                    frequency_hz=120.0,
                    rationale=detail,
                    priority=6,
                )
            )
    if not items:
        items.append(
            LLMRecommendationItem(
                action="Inspect mix balance",
                target="overview",
                frequency_hz=None,
                rationale="No DSP issues above threshold",
                priority=9,
            )
        )
    return LLMRecommendationResult(items=items)


def heuristic_harmony(features: dict[str, Any] | None) -> HarmonyProgression:
    key_info = (features or {}).get("key_estimation") or {}
    key = str(key_info.get("key") or "A minor")
    if "minor" in key.lower():
        chords = ["Am9", "Fmaj7", "Cmaj7", "Esus4"]
    else:
        chords = ["Cmaj7", "Am7", "Fmaj7", "Gsus4"]
    return HarmonyProgression(key=key, bars=8, chords=chords)
