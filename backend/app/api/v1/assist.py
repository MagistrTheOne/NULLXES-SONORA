from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.schemas.assist import AssistResponse
from app.api.deps import LLMDep, SessionDep
from app.schemas.recommendation import GenerateRecommendationRequest
from app.services import assist as assist_service

router = APIRouter(tags=["assist"])


@router.post("/assist", response_model=AssistResponse)
async def generate_assist(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> AssistResponse:
    return await assist_service.generate_assist(
        session,
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )
