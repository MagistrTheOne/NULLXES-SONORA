from __future__ import annotations

from typing import Any

from app.ai.schemas.assist import AssistOption, AssistResponse
from app.ai.schemas.clip import DropPlan, MidiClip
from app.ai.schemas.harmony import HarmonyProgression
from app.ai.schemas.recommendation import LLMRecommendationItem, LLMRecommendationResult

_FIFTH = {
    "C": "G",
    "C#": "G#",
    "DB": "AB",
    "D": "A",
    "D#": "A#",
    "EB": "BB",
    "E": "B",
    "F": "C",
    "F#": "C#",
    "GB": "DB",
    "G": "D",
    "G#": "D#",
    "AB": "EB",
    "A": "E",
    "A#": "F",
    "BB": "F",
    "B": "F#",
}


def _key_name(features: dict[str, Any] | None) -> str:
    key_info = (features or {}).get("key_estimation") or {}
    dna = (features or {}).get("dna") or {}
    identity = dna.get("identity") or {}
    identity_key = identity.get("key") or {}
    return str(key_info.get("key") or identity_key.get("name") or "A minor")


def _root_and_mode(key: str) -> tuple[str, bool]:
    text = key.strip() or "A minor"
    minor = "minor" in text.lower() or text.lower().endswith("m")
    token = text.replace("-", " ").split()[0]
    token = token.replace("min", "").replace("maj", "").replace("m", "") if len(token) > 1 else token
    root = token if token else "A"
    if len(root) >= 2 and root[1] in {"b", "B"}:
        root = root[0].upper() + "b"
    elif len(root) >= 2 and root[1] == "#":
        root = root[0].upper() + "#"
    else:
        root = root[0].upper() + root[1:]
    return root, minor


def _fifth(root: str) -> str:
    mapped = _FIFTH.get(root.upper().replace("B", "B") if "b" not in root else root.upper())
    if mapped is None:
        mapped = _FIFTH.get(root.upper(), "E")
    if mapped.endswith("B") and mapped != "B":
        return mapped[0] + "b"
    return mapped


def _pick_section(features: dict[str, Any] | None, name: str | None = None) -> dict[str, Any]:
    dna = (features or {}).get("dna") or {}
    sections = (dna.get("structure") or {}).get("sections") or []
    typed = [section for section in sections if isinstance(section, dict)]
    if name:
        matches = [section for section in typed if section.get("name") == name]
        if matches:
            return matches[0]
    if typed:
        return max(typed, key=lambda section: float(section.get("energy") or 0.0))
    duration = float((features or {}).get("duration_sec") or 0.0)
    return {"name": "body", "start": 0.0, "end": duration, "energy": 0.5}


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
    key = _key_name(features)
    _, minor = _root_and_mode(key)
    if minor:
        chords = ["Am9", "Fmaj7", "Cmaj7", "Esus4"]
    else:
        chords = ["Cmaj7", "Am7", "Fmaj7", "Gsus4"]
    return HarmonyProgression(key=key, bars=8, chords=chords)


def heuristic_bass(features: dict[str, Any] | None) -> MidiClip:
    key = _key_name(features)
    root, minor = _root_and_mode(key)
    fifth = _fifth(root)
    quality = "m" if minor else ""
    return MidiClip(
        role="bass",
        key=key,
        bars=8,
        chords=[f"{root}{quality}", f"{fifth}{quality}", f"{root}{quality}", f"{fifth}{quality}"],
        notes=[f"{root}1", f"{root}2", f"{fifth}1", f"{fifth}2"],
        pattern=["x---", "x-x-", "x---", "x-x-", "x---", "--x-", "x---", "x-x-"],
    )


def heuristic_pad(features: dict[str, Any] | None) -> MidiClip:
    key = _key_name(features)
    root, minor = _root_and_mode(key)
    fifth = _fifth(root)
    quality = "m9" if minor else "maj7"
    return MidiClip(
        role="pad",
        key=key,
        bars=8,
        chords=[f"{root}{quality}", f"{fifth}{quality}"],
        notes=[f"{root}3", f"{fifth}3", f"{root}4"],
        pattern=["xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx", "xxxx"],
    )


def heuristic_drop(features: dict[str, Any] | None) -> DropPlan:
    section = _pick_section(features, "drop")
    start = float(section.get("start") or 0.0)
    end = float(section.get("end") or start)
    name = str(section.get("name") or "drop")
    return DropPlan(
        section_name=name,
        start=round(start, 3),
        end=round(end, 3),
        actions=[
            f"Hold the first hit at {start:.1f}s, then add the extra layer.",
            "Lift presence around 3 kHz so the drop reads on small speakers.",
            "Tighten ~100 Hz so the kick punches through the bass.",
        ],
        frequency=3000.0,
        gain=2.5,
        q=1.1,
        energy_target=0.85,
    )


def heuristic_assist(
    features: dict[str, Any] | None,
    issues: list[Any] | None,
) -> AssistResponse:
    rows = [item if isinstance(item, dict) else {} for item in (issues or [])]
    types = {str(item.get("type") or "") for item in rows}
    dna = (features or {}).get("dna") or {}
    sections = (dna.get("structure") or {}).get("sections") or []
    has_drop = any(isinstance(item, dict) and item.get("name") == "drop" for item in sections)
    mix = dna.get("mix_character") or {}
    low = mix.get("low_end") or {}

    if "frequency_conflict" in types:
        headline = "Vocals need space."
        detail = "Bass and mid information are fighting. Carve a pocket, then decide if the drop still needs weight."
    elif "muddy_low_end" in types or low.get("risk") == "high":
        headline = "Bass is crowding the mix."
        detail = str(low.get("why") or "Low end is taking the centre. Tighten it before you add more.")
    elif "clipping" in types:
        headline = "Peaks are hitting the ceiling."
        detail = "Drop the input a few dB before the arrangement can open."
    elif has_drop:
        headline = "The drop can hit harder."
        detail = "Structure is there. Strengthen the landing, then write the bass that carries it."
    else:
        headline = "The track has a solid foundation."
        detail = "No urgent collision. Create the next object when you want to move."

    options = [
        AssistOption(id="strengthen_drop", label="Strengthen drop"),
        AssistOption(id="create_bass", label="Create bass"),
        AssistOption(id="fix_vocal_space", label="Fix vocal space"),
        AssistOption(id="compare_reference", label="Compare reference"),
    ]
    return AssistResponse(headline=headline, detail=detail, options=options, provider="off")
