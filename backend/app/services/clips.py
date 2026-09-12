from __future__ import annotations

from typing import Literal
from uuid import UUID

from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.factory import get_llm_provider
from app.ai.prompts.clip import build_clip_messages, build_drop_messages
from app.ai.schemas.clip import CLIP_JSON_SCHEMA, DROP_JSON_SCHEMA, DropPlan, MidiClip
from app.core.exceptions import NotFoundError
from app.services.dsp_advice import heuristic_bass, heuristic_drop, heuristic_pad
from app.services.memory import get_owner_profile
from app.services.recommendation import _resolve_analysis


async def _completed_analysis(
    session: AsyncSession,
    *,
    audio_id: UUID | None,
    analysis_id: UUID | None,
):
    analysis = await _resolve_analysis(session, audio_id=audio_id, analysis_id=analysis_id)
    if analysis.status != "completed" or not analysis.features:
        raise NotFoundError("Analysis is not completed")
    return analysis


def _owner_payload(profile) -> dict:
    return {
        "style": profile.style,
        "preferences": profile.preferences,
        "favorite_genres": profile.favorite_genres,
    }


async def generate_clip(
    session: AsyncSession,
    *,
    role: Literal["bass", "pad"],
    audio_id: UUID | None = None,
    analysis_id: UUID | None = None,
    provider: LLMProvider | None = None,
) -> MidiClip:
    llm = provider or get_llm_provider()
    analysis = await _completed_analysis(session, audio_id=audio_id, analysis_id=analysis_id)
    if llm.name == "off":
        return heuristic_bass(analysis.features) if role == "bass" else heuristic_pad(analysis.features)

    profile = await get_owner_profile(session)
    messages = build_clip_messages(
        role=role,
        analysis=analysis.features,
        issues=analysis.issues,
        owner_profile=_owner_payload(profile),
    )
    raw = await llm.complete_structured(messages, CLIP_JSON_SCHEMA)
    clip = MidiClip.model_validate(raw)
    return clip.model_copy(update={"role": role})


async def generate_drop(
    session: AsyncSession,
    *,
    audio_id: UUID | None = None,
    analysis_id: UUID | None = None,
    provider: LLMProvider | None = None,
) -> DropPlan:
    llm = provider or get_llm_provider()
    analysis = await _completed_analysis(session, audio_id=audio_id, analysis_id=analysis_id)
    if llm.name == "off":
        return heuristic_drop(analysis.features)

    profile = await get_owner_profile(session)
    messages = build_drop_messages(
        analysis=analysis.features,
        issues=analysis.issues,
        owner_profile=_owner_payload(profile),
    )
    raw = await llm.complete_structured(messages, DROP_JSON_SCHEMA)
    return DropPlan.model_validate(raw)
