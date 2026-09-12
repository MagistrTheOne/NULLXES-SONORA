from __future__ import annotations

from collections.abc import AsyncIterator

import pytest
from httpx import ASGITransport, AsyncClient
from sqlalchemy import text

from app.core.config import get_settings
from app.database.base import Base
from app.database.session import get_sync_engine, init_engines
from app.main import app
from app.services.task_runner import LocalTaskRunner
from tests.conftest import FakeLLMProvider, requires_postgres


@pytest.fixture
async def api_client(
    tmp_path, monkeypatch
) -> AsyncIterator[AsyncClient]:
    if not __import__("tests.conftest", fromlist=["postgres_available"]).postgres_available():
        pytest.skip("PostgreSQL is required for API / pipeline tests")

    monkeypatch.setenv("STORAGE_PATH", str(tmp_path / "storage"))
    monkeypatch.setenv("TASK_RUNNER", "local")
    get_settings.cache_clear()
    settings = get_settings()
    settings.storage_path.mkdir(parents=True, exist_ok=True)
    init_engines(settings)
    engine = get_sync_engine()
    Base.metadata.drop_all(engine)
    Base.metadata.create_all(engine)

    from app.api import deps

    app.dependency_overrides[deps.task_runner] = lambda: LocalTaskRunner()
    app.dependency_overrides[deps.llm_provider] = lambda: FakeLLMProvider()

    transport = ASGITransport(app=app)
    async with AsyncClient(transport=transport, base_url="http://test") as client:
        yield client

    app.dependency_overrides.clear()
    with engine.connect() as connection:
        connection.execute(text("SELECT 1"))
    Base.metadata.drop_all(engine)
    get_settings.cache_clear()


__all__ = ["api_client", "requires_postgres"]
