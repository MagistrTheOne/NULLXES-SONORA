from __future__ import annotations

import json
from typing import Any

from app.ai.base import Message

SYSTEM_PROMPT = """You are SONORA Creative Engine.
You produce MIDI-first harmonic objects, not chat and not audio.

Return ONLY a chord progression that fits the analysis key, BPM feel,
and owner style. Use lead-sheet symbols (Am9, Fmaj7, Cmaj7, Esus4).
Do not invent a different key if analysis already estimated one with
reasonable confidence. Do not write prose."""


def build_harmony_messages(
    *,
    analysis: dict[str, Any],
    owner_profile: dict[str, Any],
) -> list[Message]:
    payload = {"analysis": analysis, "owner_profile": owner_profile}
    return [
        {"role": "system", "content": SYSTEM_PROMPT},
        {
            "role": "user",
            "content": (
                "Generate an 8-bar progression as a MIDI-first object.\n\n"
                + json.dumps(payload, ensure_ascii=False)
            ),
        },
    ]
