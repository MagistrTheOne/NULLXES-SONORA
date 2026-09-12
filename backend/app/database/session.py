from collections.abc import AsyncIterator, Iterator

from sqlalchemy import create_engine
from sqlalchemy.ext.asyncio import AsyncSession, async_sessionmaker, create_async_engine
from sqlalchemy.orm import Session, sessionmaker

from app.core.config import Settings, get_settings

_async_engine = None
_async_session_factory: async_sessionmaker[AsyncSession] | None = None
_sync_engine = None
_sync_session_factory: sessionmaker[Session] | None = None


def init_engines(settings: Settings | None = None) -> None:
    global _async_engine, _async_session_factory, _sync_engine, _sync_session_factory
    settings = settings or get_settings()

    _async_engine = create_async_engine(
        settings.database_url,
        pool_pre_ping=True,
        pool_size=5,
        max_overflow=10,
    )
    _async_session_factory = async_sessionmaker(
        _async_engine,
        expire_on_commit=False,
        class_=AsyncSession,
    )

    _sync_engine = create_engine(
        settings.sync_database_url,
        pool_pre_ping=True,
        pool_size=5,
        max_overflow=10,
    )
    _sync_session_factory = sessionmaker(
        _sync_engine,
        expire_on_commit=False,
        class_=Session,
    )


def get_async_engine():
    if _async_engine is None:
        init_engines()
    return _async_engine


def get_sync_engine():
    if _sync_engine is None:
        init_engines()
    return _sync_engine


def async_session_factory() -> async_sessionmaker[AsyncSession]:
    if _async_session_factory is None:
        init_engines()
    assert _async_session_factory is not None
    return _async_session_factory


def sync_session_factory() -> sessionmaker[Session]:
    if _sync_session_factory is None:
        init_engines()
    assert _sync_session_factory is not None
    return _sync_session_factory


async def get_async_session() -> AsyncIterator[AsyncSession]:
    factory = async_session_factory()
    async with factory() as session:
        yield session


def get_sync_session() -> Iterator[Session]:
    factory = sync_session_factory()
    with factory() as session:
        yield session
