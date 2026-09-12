from fastapi import APIRouter

from app.api.v1.audio import router as audio_router
from app.api.v1.events import router as events_router
from app.api.v1.memory import router as memory_router
from app.api.v1.profile import router as profile_router
from app.api.v1.recommendation import router as recommendation_router

api_v1_router = APIRouter(prefix="/api/v1")
api_v1_router.include_router(audio_router)
api_v1_router.include_router(recommendation_router)
api_v1_router.include_router(profile_router)
api_v1_router.include_router(memory_router)
api_v1_router.include_router(events_router)
