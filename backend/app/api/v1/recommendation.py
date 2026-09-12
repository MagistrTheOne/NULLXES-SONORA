from uuid import UUID

from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.api.deps import LLMDep, SessionDep
from app.models.recommendation import Recommendation
from app.schemas.recommendation import (
    DecisionCreate,
    DecisionOut,
    GenerateRecommendationRequest,
    RecommendationItem,
    RecommendationOut,
)
from app.services import memory as memory_service
from app.services import recommendation as recommendation_service

router = APIRouter(tags=["recommendation"])


def _to_out(row: Recommendation) -> RecommendationOut:
    items = [RecommendationItem.model_validate(item) for item in row.items]
    return RecommendationOut(
        id=row.id,
        analysis_id=row.analysis_id,
        audio_id=row.audio_id,
        provider=row.provider,
        model=row.model,
        items=items,
        recommendations=[item.action for item in items],
        created_at=row.created_at,
    )


@router.post("/recommendation/generate", response_model=RecommendationOut)
async def generate_recommendation(
    body: GenerateRecommendationRequest,
    session: AsyncSession = SessionDep,
    provider: LLMProvider = LLMDep,
) -> RecommendationOut:
    row = await recommendation_service.generate_recommendations(
        session,
        audio_id=body.audio_id,
        analysis_id=body.analysis_id,
        provider=provider,
    )
    assert isinstance(row, Recommendation)
    return _to_out(row)


@router.post(
    "/recommendation/{recommendation_id}/decision",
    response_model=DecisionOut,
    status_code=201,
)
async def record_decision(
    recommendation_id: UUID,
    body: DecisionCreate,
    session: AsyncSession = SessionDep,
) -> DecisionOut:
    row = await memory_service.record_decision(
        session,
        recommendation_id=recommendation_id,
        decision=body.decision,
        item_index=body.item_index,
        note=body.note,
    )
    return DecisionOut.model_validate(row)
