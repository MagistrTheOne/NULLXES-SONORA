from datetime import datetime
from uuid import UUID

from pydantic import BaseModel, ConfigDict, Field


class ORMModel(BaseModel):
    model_config = ConfigDict(from_attributes=True)


class HealthResponse(BaseModel):
    status: str
    service: str = "sonora"
    database: str


class ErrorResponse(BaseModel):
    code: str
    message: str


class EnqueuedAnalysisResponse(BaseModel):
    analysis_id: UUID
    audio_id: UUID
    status: str = "pending"


class EventOut(ORMModel):
    id: int
    type: str
    entity_type: str
    entity_id: str
    payload: dict = Field(default_factory=dict)
    created_at: datetime


class EventListResponse(BaseModel):
    items: list[EventOut]
    next_since_id: int | None = None
