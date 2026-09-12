from datetime import datetime

from fastapi import APIRouter, Depends, Query
from sqlalchemy.ext.asyncio import AsyncSession

from app.api.deps import db_session
from app.schemas.common import EventListResponse, EventOut
from app.services.events import EventService

router = APIRouter(prefix="/events", tags=["events"])


@router.get("", response_model=EventListResponse)
async def poll_events(
    since_id: int = Query(default=0, ge=0),
    after: datetime | None = Query(default=None),
    limit: int = Query(default=100, ge=1, le=500),
    session: AsyncSession = Depends(db_session),
) -> EventListResponse:
    items = await EventService.list_since(
        session, since_id=since_id, after=after, limit=limit
    )
    next_id = items[-1].id if items else None
    return EventListResponse(
        items=[EventOut.model_validate(item) for item in items],
        next_since_id=next_id,
    )
