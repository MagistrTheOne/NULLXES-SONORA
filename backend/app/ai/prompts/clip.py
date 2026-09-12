from __future__ import annotations

import json
from typing import Any, Literal

from app.ai.base import Message
from app.ai.prompts.context import compact_track

BASS_SYSTEM = """You are SONORA Creative Engine.
You produce MIDI-first bass objects, not chat and not audio.

Return a house/club bass clip in the analysis key.
notes are pitch names with octave (A1, E2).
pattern cells are one bar of 16ths using x (hit) and - (rest), e.g. x---.
Do not invent a different key if analysis already estimated one.
Do not write prose."""

PAD_SYSTEM = """You are SONORA Creative Engine.
You produce MIDI-first pad objects, not chat and not audio.

Return an open-voiced pad in the analysis key.
notes are pitch names with octave (A3, E3, A4).
pattern cells are sustained bars (xxxx).
Do not invent a different key if analysis already estimated one.
Do not write prose."""

DROP_SYSTEM = """You are SONORA Creative Engine.
You produce a drop plan object, not chat and not audio.

Use the detected drop section times when present.
Give concrete mix actions a musician can apply.
frequency/gain/q describe one EQ move that strengthens the drop
(presence lift near 3k or a tight cut near 100Hz).
energy_target is 0-1. Do not write prose."""


def build_clip_messages(
    *,
    role: Literal["bass", "pad"],
    analysis: dict[str, Any],
    issues: list[Any] | None,
    owner_profile: dict[str, Any],
) -> list[Message]:
    system = BASS_SYSTEM if role == "bass" else PAD_SYSTEM
    payload = {
        "role": role,
        "track": compact_track(analysis, issues),
        "owner_profile": owner_profile,
    }
    return [
        {"role": "system", "content": system},
        {
            "role": "user",
            "content": (
                f"Generate an 8-bar {role} MIDI object.\n\n"
                + json.dumps(payload, ensure_ascii=False)
            ),
        },
    ]


def build_drop_messages(
    *,
    analysis: dict[str, Any],
    issues: list[Any] | None,
    owner_profile: dict[str, Any],
) -> list[Message]:
    payload = {
        "track": compact_track(analysis, issues),
        "owner_profile": owner_profile,
    }
    return [
        {"role": "system", "content": DROP_SYSTEM},
        {
            "role": "user",
            "content": (
                "Strengthen the drop. Return the plan object.\n\n"
                + json.dumps(payload, ensure_ascii=False)
            ),
        },
    ]
