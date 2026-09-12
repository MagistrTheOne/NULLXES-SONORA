from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.api.deps import SessionDep
from app.schemas.audio import AnalysisOut
from app.schemas.memory import MemoryResponse
from app.schemas.recommendation import DecisionOut
from app.services import memory as memory_service

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
