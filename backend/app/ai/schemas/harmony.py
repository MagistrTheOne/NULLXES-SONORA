from __future__ import annotations

from pydantic import BaseModel, Field

HARMONY_JSON_SCHEMA: dict = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "key": {"type": "string"},
        "bars": {"type": "integer", "minimum": 1, "maximum": 32},
        "chords": {
            "type": "array",
            "items": {"type": "string"},
            "minItems": 4,
            "maxItems": 16,
        },
    },
    "required": ["key", "bars", "chords"],
}


class HarmonyProgression(BaseModel):
    key: str
    bars: int = Field(ge=1, le=32)
    chords: list[str]
