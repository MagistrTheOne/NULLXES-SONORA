from __future__ import annotations

from pydantic import BaseModel

ASSIST_JSON_SCHEMA: dict = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "headline": {"type": "string"},
        "detail": {"type": "string"},
        "options": {
            "type": "array",
            "items": {
                "type": "object",
                "additionalProperties": False,
                "properties": {
                    "id": {"type": "string"},
                    "label": {"type": "string"},
                },
                "required": ["id", "label"],
            },
            "minItems": 1,
            "maxItems": 5,
        },
    },
    "required": ["headline", "detail", "options"],
}


class AssistOption(BaseModel):
    id: str
    label: str


class AssistResponse(BaseModel):
    headline: str
    detail: str
    options: list[AssistOption]
    provider: str = "off"
