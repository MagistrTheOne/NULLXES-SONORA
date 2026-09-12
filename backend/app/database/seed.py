import logging

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session

from app.models.owner_profile import OWNER_PROFILE_ID, OwnerProfile

logger = logging.getLogger(__name__)

DEFAULT_PREFERENCES: dict[str, str] = {
    "bass": "balanced",
    "vocals": "clean",
    "mix": "wide",
}


async def seed_owner_profile(session: AsyncSession) -> OwnerProfile:
    result = await session.execute(
        select(OwnerProfile).where(OwnerProfile.id == OWNER_PROFILE_ID)
    )
    profile = result.scalar_one_or_none()
    if profile is not None:
        return profile

    profile = OwnerProfile(
        id=OWNER_PROFILE_ID,
        style=[],
        preferences=dict(DEFAULT_PREFERENCES),
        favorite_genres=[],
        preferred_sound_profile={},
    )
    session.add(profile)
    await session.commit()
    await session.refresh(profile)
    logger.info("Seeded singleton owner_profile id=%s", OWNER_PROFILE_ID)
    return profile


def seed_owner_profile_sync(session: Session) -> OwnerProfile:
    profile = session.get(OwnerProfile, OWNER_PROFILE_ID)
    if profile is not None:
        return profile
    profile = OwnerProfile(
        id=OWNER_PROFILE_ID,
        style=[],
        preferences=dict(DEFAULT_PREFERENCES),
        favorite_genres=[],
        preferred_sound_profile={},
    )
    session.add(profile)
    session.commit()
    session.refresh(profile)
    return profile
