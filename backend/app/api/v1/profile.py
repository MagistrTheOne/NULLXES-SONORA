from fastapi import APIRouter
from sqlalchemy.ext.asyncio import AsyncSession

from app.api.deps import SessionDep
from app.schemas.profile import OwnerProfileOut, OwnerProfileUpdate
from app.services import memory as memory_service

router = APIRouter(prefix="/profile", tags=["profile"])


@router.get("", response_model=OwnerProfileOut)
async def get_profile(session: AsyncSession = SessionDep) -> OwnerProfileOut:
    profile = await memory_service.get_owner_profile(session)
    return OwnerProfileOut.model_validate(profile)


@router.put("", response_model=OwnerProfileOut)
async def put_profile(
    body: OwnerProfileUpdate,
    session: AsyncSession = SessionDep,
) -> OwnerProfileOut:
    profile = await memory_service.update_owner_profile(
        session,
        style=body.style,
        preferences=body.preferences,
        favorite_genres=body.favorite_genres,
        preferred_sound_profile=body.preferred_sound_profile,
    )
    return OwnerProfileOut.model_validate(profile)
