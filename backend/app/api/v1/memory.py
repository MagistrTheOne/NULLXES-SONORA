from datetime import datetime

from fastapi import APIRouter, Query
from sqlalchemy.ext.asyncio import AsyncSession

from app.api.deps import SessionDep
from app.schemas.audio import AnalysisOut
from app.schemas.common import EventListResponse, EventOut
from app.schemas.memory import MemoryResponse
from app.schemas.recommendation import DecisionOut
from app.services import memory as memory_service
from app.services.events import EventService

router = APIRouter(tags=["memory"])


@router.get("/memory", response_model=MemoryResponse)
async def get_memory(session: AsyncSession = SessionDep) -> MemoryResponse:
    profile = await memory_service.get_owner_profile(session)
    analyses = await memory_service.recent_analyses(session)
    decisions = await memory_service.recent_decisions(session)
    return MemoryResponse(
        preferred_sound_profile=profile.preferred_sound_profile,
        analyses=[AnalysisOut.model_validate(item) for item in analyses],
        decisions=[DecisionOut.model_validate(item) for item in decisions],
    )


@router.get("/events", response_model=EventListResponse)
async def get_events(
    session: AsyncSession = SessionDep,
    since_id: int = Query(default=0, ge=0),
    after: datetime | None = Query(default=None),
    limit: int = Query(default=100, ge=1, le=500),
) -> EventListResponse:
    items = await EventService.list_since(
        session, since_id=since_id, limit=limit, after=after
    )
    next_id = items[-1].id if items else None
    return EventListResponse(
        items=[EventOut.model_validate(item) for item in items],
        next_since_id=next_id,
    )
