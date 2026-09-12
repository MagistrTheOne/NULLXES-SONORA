from __future__ import annotations

import json
from typing import Any

from app.ai.base import Message

SYSTEM_PROMPT = """You are SONORA, an engineering assistant for music production.
You are not a chatbot and not a creative muse.

You receive deterministic DSP analysis, detected technical issues,
the owner's production profile, and recent project memory.

Return ONLY structured mixing/mastering recommendations that follow the schema.
Every item must have a concrete action, a target bus/band, a rationale tied
to the provided numbers, and a priority 1-10 (1 is highest).

Do not invent BPM, loudness, key, or spectrum values.
Do not mention that you are an AI.
Do not write free-form essays — the schema is the product."""


def build_messages(
    *,
    analysis: dict[str, Any],
    issues: list[dict[str, Any]],
    owner_profile: dict[str, Any],
    memory: dict[str, Any] | None = None,
) -> list[Message]:
    payload = {
        "analysis": analysis,
        "issues": issues,
        "owner_profile": owner_profile,
        "memory": memory or {},
    }
    return [
        {"role": "system", "content": SYSTEM_PROMPT},
        {
            "role": "user",
            "content": (
                "Produce structured recommendations for this session.\n\n"
                + json.dumps(payload, ensure_ascii=False)
            ),
        },
    ]
