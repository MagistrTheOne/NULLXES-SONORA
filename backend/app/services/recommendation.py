from __future__ import annotations

from uuid import UUID

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.factory import get_llm_provider
from app.ai.prompts.mixing_engineer import build_messages
from app.ai.schemas.recommendation import (
    LLMRecommendationResult,
    RECOMMENDATION_JSON_SCHEMA,
)
from app.core.exceptions import NotFoundError
from app.models.analysis import Analysis
from app.models.recommendation import Recommendation
from app.services.events import EventService
from app.services.memory import get_owner_profile, memory_context


async def _resolve_analysis(
    session: AsyncSession,
    *,
    audio_id: UUID | None,
    analysis_id: UUID | None,
) -> Analysis:
    if analysis_id is not None:
        analysis = await session.get(Analysis, analysis_id)
        if analysis is None:
            raise NotFoundError(f"Analysis {analysis_id} not found")
        return analysis

    assert audio_id is not None
    result = await session.execute(
        select(Analysis)
        .where(Analysis.audio_id == audio_id, Analysis.status == "completed")
        .order_by(Analysis.created_at.desc())
        .limit(1)
    )
    analysis = result.scalar_one_or_none()
    if analysis is None:
        raise NotFoundError(f"No completed analysis for audio {audio_id}")
    return analysis


async def generate_recommendations(
    session: AsyncSession,
    *,
    audio_id: UUID | None = None,
    analysis_id: UUID | None = None,
    provider: LLMProvider | None = None,
    analysis_override: dict | None = None,
    persist: bool = True,
) -> Recommendation | LLMRecommendationResult:
    llm = provider or get_llm_provider()
    profile = await get_owner_profile(session)
    memory = await memory_context(session)

    if analysis_override is not None:
        features = analysis_override.get("features") or analysis_override
        issues = analysis_override.get("issues") or []
        messages = build_messages(
            analysis=features,
            issues=issues,
            owner_profile={
                "style": profile.style,
                "preferences": profile.preferences,
                "favorite_genres": profile.favorite_genres,
                "preferred_sound_profile": profile.preferred_sound_profile,
            },
            memory=memory,
        )
        raw = await llm.complete_structured(messages, RECOMMENDATION_JSON_SCHEMA)
        return LLMRecommendationResult.model_validate(raw)

    analysis = await _resolve_analysis(
        session, audio_id=audio_id, analysis_id=analysis_id
    )
    if analysis.status != "completed" or not analysis.features:
        raise NotFoundError("Analysis is not completed")

    messages = build_messages(
        analysis=analysis.features,
        issues=analysis.issues or [],
        owner_profile={
            "style": profile.style,
            "preferences": profile.preferences,
            "favorite_genres": profile.favorite_genres,
            "preferred_sound_profile": profile.preferred_sound_profile,
        },
        memory=memory,
    )
    raw = await llm.complete_structured(messages, RECOMMENDATION_JSON_SCHEMA)
    parsed = LLMRecommendationResult.model_validate(raw)

    if not persist:
        return parsed

    row = Recommendation(
        analysis_id=analysis.id,
        audio_id=analysis.audio_id,
        provider=llm.name,
        model=llm.model,
        items=[item.model_dump() for item in parsed.items],
    )
    session.add(row)
    await session.flush()
    await EventService.append(
        session,
        event_type="recommendation_generated",
        entity_type="audio",
        entity_id=analysis.audio_id,
        payload={
            "recommendation_id": str(row.id),
            "analysis_id": str(analysis.id),
            "provider": llm.name,
            "model": llm.model,
        },
    )
    await session.commit()
    await session.refresh(row)
    return row
