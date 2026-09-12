from __future__ import annotations

import json
from typing import Any

from app.ai.base import Message
from app.ai.prompts.context import compact_track

SYSTEM_PROMPT = """You are SONORA Assist — an engineering advisor, not a chatbot.

Speak like a mix engineer in two short sentences: headline, then detail.
Offer 2-4 concrete options a musician can run now.
Option ids MUST be from this set:
- strengthen_drop
- create_bass
- create_pad
- fix_vocal_space
- compare_reference
- create_chords

Do not invent DSP facts that contradict the analysis.
Do not dump methods or raw numbers unless they help a decision.
Do not write a conversation."""


def build_assist_messages(
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
        {"role": "system", "content": SYSTEM_PROMPT},
        {
            "role": "user",
            "content": (
                "What should we do next on this track?\n\n"
                + json.dumps(payload, ensure_ascii=False)
            ),
        },
    ]
