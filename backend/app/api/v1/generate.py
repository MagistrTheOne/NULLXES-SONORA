from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.schemas.harmony import HarmonyProgression
from app.api.deps import LLMDep, SessionDep
from app.schemas.recommendation import GenerateRecommendationRequest
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
