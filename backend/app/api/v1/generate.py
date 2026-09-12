from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.schemas.clip import DropPlan, MidiClip
from app.ai.schemas.harmony import HarmonyProgression
from app.api.deps import LLMDep, SessionDep
from app.schemas.recommendation import GenerateRecommendationRequest
from app.services import clips as clips_service
from app.services import harmony as harmony_service

router = APIRouter(prefix="/generate", tags=["generate"])


@router.post("/harmony", response_model=HarmonyProgression)
async def generate_harmony(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> HarmonyProgression:
    return await harmony_service.generate_harmony(
        session,
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )


@router.post("/bass", response_model=MidiClip)
async def generate_bass(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> MidiClip:
    return await clips_service.generate_clip(
        session,
        role="bass",
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )


@router.post("/pad", response_model=MidiClip)
async def generate_pad(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> MidiClip:
    return await clips_service.generate_clip(
        session,
        role="pad",
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )


@router.post("/drop", response_model=DropPlan)
async def generate_drop(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> DropPlan:
    return await clips_service.generate_drop(
        session,
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )
