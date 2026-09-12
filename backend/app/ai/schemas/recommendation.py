from __future__ import annotations

from pydantic import BaseModel, Field

RECOMMENDATION_JSON_SCHEMA: dict = {
    "type": "object",
    "additionalProperties": False,
    "properties": {
        "items": {
            "type": "array",
            "items": {
                "type": "object",
                "additionalProperties": False,
                "properties": {
                    "action": {"type": "string"},
                    "target": {"type": "string"},
                    "frequency_hz": {"type": ["number", "null"]},
                    "rationale": {"type": "string"},
                    "priority": {"type": "integer", "minimum": 1, "maximum": 10},
                },
                "required": [
                    "action",
                    "target",
                    "frequency_hz",
                    "rationale",
                    "priority",
                ],
            },
        }
    },
    "required": ["items"],
}


class LLMRecommendationItem(BaseModel):
    action: str
    target: str
    frequency_hz: float | None = None
    rationale: str
    priority: int = Field(ge=1, le=10)


class LLMRecommendationResult(BaseModel):
    items: list[LLMRecommendationItem]
