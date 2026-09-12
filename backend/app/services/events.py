from __future__ import annotations

from datetime import datetime
from typing import Any
from uuid import UUID

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session

from app.models.system_event import SystemEvent


def _entity_id(entity_id: UUID | str | int) -> str:
    return str(entity_id)


class EventService:
    @staticmethod
    async def append(
        session: AsyncSession,
        *,
        event_type: str,
        entity_type: str,
        entity_id: UUID | str | int,
        payload: dict[str, Any] | None = None,
    ) -> SystemEvent:
        event = SystemEvent(
            type=event_type,
            entity_type=entity_type,
            entity_id=_entity_id(entity_id),
            payload=payload or {},
        )
        session.add(event)
        await session.flush()
        return event

    @staticmethod
    def append_sync(
        session: Session,
        *,
        event_type: str,
        entity_type: str,
        entity_id: UUID | str | int,
        payload: dict[str, Any] | None = None,
    ) -> SystemEvent:
        event = SystemEvent(
            type=event_type,
            entity_type=entity_type,
            entity_id=_entity_id(entity_id),
            payload=payload or {},
        )
        session.add(event)
        session.flush()
        return event

    @staticmethod
    async def list_since(
        session: AsyncSession,
        *,
        since_id: int = 0,
        limit: int = 100,
        after: datetime | None = None,
    ) -> list[SystemEvent]:
        stmt = select(SystemEvent).order_by(SystemEvent.id.asc()).limit(limit)
        if since_id > 0:
            stmt = stmt.where(SystemEvent.id > since_id)
        if after is not None:
            stmt = stmt.where(SystemEvent.created_at > after)
        result = await session.execute(stmt)
        return list(result.scalars().all())
