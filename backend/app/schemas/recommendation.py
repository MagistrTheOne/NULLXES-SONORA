from datetime import datetime
from typing import Literal
from uuid import UUID

from pydantic import BaseModel, Field, model_validator

from app.schemas.common import ORMModel


class RecommendationItem(BaseModel):
    action: str
    target: str
    frequency_hz: float | None = None
    rationale: str
    priority: int = Field(ge=1, le=10)


class GenerateRecommendationRequest(BaseModel):
    audio_id: UUID | None = None
    analysis_id: UUID | None = None

    @model_validator(mode="after")
    def require_one_id(self) -> "GenerateRecommendationRequest":
        if self.audio_id is None and self.analysis_id is None:
            raise ValueError("audio_id or analysis_id is required")
        return self


class RecommendationOut(ORMModel):
    id: UUID
    analysis_id: UUID
    audio_id: UUID
    provider: str
    model: str
    items: list[RecommendationItem]
    recommendations: list[str]
    created_at: datetime


class DecisionCreate(BaseModel):
    decision: Literal["accepted", "rejected", "modified"]
    item_index: int | None = None
    note: str | None = None


class DecisionOut(ORMModel):
    id: UUID
    recommendation_id: UUID
    item_index: int | None
    decision: str
    note: str | None
    created_at: datetime
