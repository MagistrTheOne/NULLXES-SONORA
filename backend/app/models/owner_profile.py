from datetime import datetime

from sqlalchemy import DateTime, Integer, func
from sqlalchemy.orm import Mapped, mapped_column

from app.database.base import Base
from app.database.types import JSONType

OWNER_PROFILE_ID = 1


class OwnerProfile(Base):
    __tablename__ = "owner_profiles"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    style: Mapped[list] = mapped_column(JSONType, nullable=False, default=list)
    preferences: Mapped[dict] = mapped_column(JSONType, nullable=False, default=dict)
    favorite_genres: Mapped[list] = mapped_column(JSONType, nullable=False, default=list)
    preferred_sound_profile: Mapped[dict] = mapped_column(
        JSONType, nullable=False, default=dict
    )
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
        nullable=False,
    )
