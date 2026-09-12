import uuid
from datetime import datetime

from sqlalchemy import DateTime, ForeignKey, String, Text, func
from sqlalchemy.orm import Mapped, mapped_column, relationship

from app.database.base import Base
from app.database.types import JSONType, UUIDType


class Analysis(Base):
    __tablename__ = "analyses"

    id: Mapped[uuid.UUID] = mapped_column(
        UUIDType, primary_key=True, default=uuid.uuid4
    )
    audio_id: Mapped[uuid.UUID] = mapped_column(
        UUIDType,
        ForeignKey("audio_assets.id", ondelete="CASCADE"),
        nullable=False,
        index=True,
    )
    status: Mapped[str] = mapped_column(String(32), nullable=False, default="pending")
    analyzer_version: Mapped[str] = mapped_column(String(64), nullable=False)
    features: Mapped[dict | None] = mapped_column(JSONType, nullable=True)
    issues: Mapped[list | None] = mapped_column(JSONType, nullable=True)
    error: Mapped[str | None] = mapped_column(Text, nullable=True)
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True), server_default=func.now(), nullable=False
    )
    completed_at: Mapped[datetime | None] = mapped_column(
        DateTime(timezone=True), nullable=True
    )

    audio = relationship("AudioAsset", back_populates="analyses")
    recommendations = relationship(
        "Recommendation", back_populates="analysis", cascade="all, delete-orphan"
    )
