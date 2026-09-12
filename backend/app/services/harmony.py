from __future__ import annotations

from uuid import UUID

from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.factory import get_llm_provider
from app.ai.prompts.harmony import build_harmony_messages
from app.ai.schemas.harmony import HARMONY_JSON_SCHEMA, HarmonyProgression
from app.core.exceptions import NotFoundError
from app.services.memory import get_owner_profile
from app.services.recommendation import _resolve_analysis


async def generate_harmony(
    session: AsyncSession,
    *,
    audio_id: UUID | None = None,
    analysis_id: UUID | None = None,
    provider: LLMProvider | None = None,
) -> HarmonyProgression:
    llm = provider or get_llm_provider()
    analysis = await _resolve_analysis(session, audio_id=audio_id, analysis_id=analysis_id)
    if analysis.status != "completed" or not analysis.features:
        raise NotFoundError("Analysis is not completed")

    profile = await get_owner_profile(session)
    messages = build_harmony_messages(
        analysis=analysis.features,
        owner_profile={
            "style": profile.style,
            "preferences": profile.preferences,
            "favorite_genres": profile.favorite_genres,
        },
    )
    raw = await llm.complete_structured(messages, HARMONY_JSON_SCHEMA)
    return HarmonyProgression.model_validate(raw)
