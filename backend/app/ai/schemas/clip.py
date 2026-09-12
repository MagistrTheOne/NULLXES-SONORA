from __future__ import annotations

from typing import Literal

from pydantic import BaseModel, Field

CLIP_JSON_SCHEMA: dict = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "role": {"type": "string", "enum": ["harmony", "bass", "pad"]},
        "key": {"type": "string"},
        "bars": {"type": "integer", "minimum": 1, "maximum": 32},
        "chords": {"type": "array", "items": {"type": "string"}},
        "notes": {"type": "array", "items": {"type": "string"}},
        "pattern": {"type": "array", "items": {"type": "string"}},
    },
    "required": ["role", "key", "bars", "chords", "notes", "pattern"],
}

DROP_JSON_SCHEMA: dict = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "section_name": {"type": "string"},
        "start": {"type": "number"},
        "end": {"type": "number"},
        "actions": {"type": "array", "items": {"type": "string"}, "minItems": 1},
        "frequency": {"type": "number"},
        "gain": {"type": "number"},
        "q": {"type": "number"},
        "energy_target": {"type": "number"},
    },
    "required": [
        "section_name",
        "start",
        "end",
        "actions",
        "frequency",
        "gain",
        "q",
        "energy_target",
    ],
}


class MidiClip(BaseModel):
    role: Literal["harmony", "bass", "pad"]
    key: str
    bars: int = Field(ge=1, le=32)
    chords: list[str] = Field(default_factory=list)
    notes: list[str] = Field(default_factory=list)
    pattern: list[str] = Field(default_factory=list)


class DropPlan(BaseModel):
    section_name: str
    start: float
    end: float
    actions: list[str]
    frequency: float
    gain: float
    q: float
    energy_target: float
