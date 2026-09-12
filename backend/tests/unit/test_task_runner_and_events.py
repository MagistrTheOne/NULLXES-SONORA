from __future__ import annotations

from pathlib import Path

import pytest
from sqlalchemy import select

from app.core.config import get_settings
from app.database.base import Base
from app.database.seed import seed_owner_profile
from app.database.session import async_session_factory, get_async_engine, init_engines
from app.models.system_event import SystemEvent
from app.services.analysis import create_asset_from_bytes, enqueue_analysis
from app.services.task_runner import LocalTaskRunner
from tests.conftest import requires_postgres, write_stereo_wav


@pytest.fixture
async def db_ready(tmp_path: Path, monkeypatch: pytest.MonkeyPatch):
    monkeypatch.setenv("STORAGE_PATH", str(tmp_path))
    monkeypatch.setenv("TASK_RUNNER", "local")
    get_settings.cache_clear()
    init_engines()
    engine = get_async_engine()
    async with engine.begin() as connection:
        await connection.run_sync(Base.metadata.create_all)
    factory = async_session_factory()
    async with factory() as session:
        await seed_owner_profile(session)
    yield
    async with engine.begin() as connection:
        await connection.run_sync(Base.metadata.drop_all)
    get_settings.cache_clear()


@requires_postgres
@pytest.mark.asyncio
async def test_local_task_runner_completes_analysis_and_emits_event(
    db_ready, tmp_path: Path
) -> None:
    wav = write_stereo_wav(tmp_path / "track.wav")
    factory = async_session_factory()
    runner = LocalTaskRunner()
    async with factory() as session:
        asset = await create_asset_from_bytes(
            session, filename=wav.name, data=wav.read_bytes()
        )
        analysis = await enqueue_analysis(session, runner, audio_id=asset.id)
        await session.refresh(analysis)
        assert analysis.status == "completed"
        assert analysis.analyzer_version.startswith("SONORA_DSP")
        assert analysis.features is not None
        assert "loudness_lufs_approx" in analysis.features
        assert analysis.features["key_estimation"]["method"] == "chroma_cqt"

        events = (
            await session.execute(
                select(SystemEvent).where(SystemEvent.type == "analysis_completed")
            )
        ).scalars().all()
        assert any(event.entity_id == str(asset.id) for event in events)

    latest = tmp_path / "audio" / str(asset.id) / "analysis" / "latest.json"
    assert latest.exists()
