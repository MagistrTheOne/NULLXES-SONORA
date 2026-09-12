from __future__ import annotations

from uuid import UUID

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.core.exceptions import NotFoundError
from app.models.analysis import Analysis
from app.models.owner_decision import OwnerDecision
from app.models.owner_profile import OWNER_PROFILE_ID, OwnerProfile
from app.models.recommendation import Recommendation
from app.services.events import EventService


async def get_owner_profile(session: AsyncSession) -> OwnerProfile:
    profile = await session.get(OwnerProfile, OWNER_PROFILE_ID)
    if profile is None:
        raise NotFoundError("owner_profile is not seeded")
    return profile


async def update_owner_profile(
    session: AsyncSession,
    *,
    style: list[str] | None = None,
    preferences: dict | None = None,
    favorite_genres: list[str] | None = None,
    preferred_sound_profile: dict | None = None,
) -> OwnerProfile:
    profile = await get_owner_profile(session)
    if style is not None:
        profile.style = style
    if preferences is not None:
        profile.preferences = preferences
    if favorite_genres is not None:
        profile.favorite_genres = favorite_genres
    if preferred_sound_profile is not None:
        profile.preferred_sound_profile = preferred_sound_profile
    await session.commit()
    await session.refresh(profile)
    return profile


async def recent_analyses(session: AsyncSession, limit: int = 10) -> list[Analysis]:
    result = await session.execute(
        select(Analysis)
        .where(Analysis.status == "completed")
        .order_by(Analysis.created_at.desc())
        .limit(limit)
    )
    return list(result.scalars().all())


async def recent_decisions(session: AsyncSession, limit: int = 20) -> list[OwnerDecision]:
    result = await session.execute(
        select(OwnerDecision).order_by(OwnerDecision.created_at.desc()).limit(limit)
    )
    return list(result.scalars().all())


async def memory_context(session: AsyncSession) -> dict:
    profile = await get_owner_profile(session)
    analyses = await recent_analyses(session, limit=5)
    decisions = await recent_decisions(session, limit=10)
    return {
        "preferred_sound_profile": profile.preferred_sound_profile,
        "recent_analyses": [
            {
                "id": str(item.id),
                "audio_id": str(item.audio_id),
                "analyzer_version": item.analyzer_version,
                "issues": item.issues or [],
            }
            for item in analyses
        ],
        "recent_decisions": [
            {
                "decision": item.decision,
                "item_index": item.item_index,
                "note": item.note,
            }
            for item in decisions
        ],
    }


async def record_decision(
    session: AsyncSession,
    *,
    recommendation_id: UUID,
    decision: str,
    item_index: int | None,
    note: str | None,
) -> OwnerDecision:
    recommendation = await session.get(Recommendation, recommendation_id)
    if recommendation is None:
        raise NotFoundError(f"Recommendation {recommendation_id} not found")
    row = OwnerDecision(
        recommendation_id=recommendation_id,
        item_index=item_index,
        decision=decision,
        note=note,
    )
    session.add(row)
    await EventService.append(
        session,
        event_type="decision_recorded",
        entity_type="recommendation",
        entity_id=recommendation_id,
        payload={"decision": decision, "item_index": item_index},
    )
    await session.commit()
    await session.refresh(row)
    return row
