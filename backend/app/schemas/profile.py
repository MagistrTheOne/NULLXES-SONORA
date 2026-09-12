from datetime import datetime

from pydantic import BaseModel, Field

from app.schemas.common import ORMModel


class OwnerPreferences(BaseModel):
    bass: str = "balanced"
    vocals: str = "clean"
    mix: str = "wide"

    model_config = {"extra": "allow"}


class OwnerProfileOut(ORMModel):
    id: int
    style: list[str]
    preferences: dict
    favorite_genres: list[str]
    preferred_sound_profile: dict
    updated_at: datetime


class OwnerProfileUpdate(BaseModel):
    style: list[str] | None = None
    preferences: dict | None = None
    favorite_genres: list[str] | None = None
    preferred_sound_profile: dict | None = Field(default=None)
