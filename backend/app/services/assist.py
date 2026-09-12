from __future__ import annotations

from uuid import UUID

from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.factory import get_llm_provider
from app.ai.prompts.assist import build_assist_messages
from app.ai.schemas.assist import ASSIST_JSON_SCHEMA, AssistResponse
from app.core.exceptions import NotFoundError
from app.services.dsp_advice import heuristic_assist
from app.services.memory import get_owner_profile
from app.services.recommendation import _resolve_analysis


async def generate_assist(
    session: AsyncSession,
    *,
    audio_id: UUID | None = None,
    analysis_id: UUID | None = None,
    provider: LLMProvider | None = None,
) -> AssistResponse:
    llm = provider or get_llm_provider()
    analysis = await _resolve_analysis(session, audio_id=audio_id, analysis_id=analysis_id)
    if analysis.status != "completed" or not analysis.features:
        raise NotFoundError("Analysis is not completed")

    if llm.name == "off":
        return heuristic_assist(analysis.features, analysis.issues)

    profile = await get_owner_profile(session)
    messages = build_assist_messages(
        analysis=analysis.features,
        issues=analysis.issues,
        owner_profile={
            "style": profile.style,
            "preferences": profile.preferences,
            "favorite_genres": profile.favorite_genres,
        },
    )
    raw = await llm.complete_structured(messages, ASSIST_JSON_SCHEMA)
    advice = AssistResponse.model_validate(raw)
    return advice.model_copy(update={"provider": llm.name})
