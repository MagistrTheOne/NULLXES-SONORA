from __future__ import annotations

import logging
from datetime import UTC, datetime
from pathlib import Path
from uuid import UUID, uuid4

from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session

from app.audio.dsp.features import extract_features
from app.audio.intelligence.issues import detect_issues
from app.audio.loader import load_audio
from app.core.config import get_settings
from app.core.exceptions import AnalysisError, NotFoundError, PayloadTooLargeError
from app.models.analysis import Analysis
from app.models.audio_asset import AudioAsset
from app.services.events import EventService
from app.services.storage import get_storage
from app.services.task_runner import TaskRunner

logger = logging.getLogger(__name__)

MIME_BY_EXT = {
    "wav": "audio/wav",
    "mp3": "audio/mpeg",
    "flac": "audio/flac",
}


def _mime(filename: str) -> str:
    ext = Path(filename).suffix.lstrip(".").lower()
    return MIME_BY_EXT.get(ext, "application/octet-stream")


async def _get_asset(session: AsyncSession, audio_id: UUID) -> AudioAsset:
    asset = await session.get(AudioAsset, audio_id)
    if asset is None:
        raise NotFoundError(f"Audio asset {audio_id} not found")
    return asset


async def create_asset_from_bytes(
    session: AsyncSession,
    *,
    filename: str,
    data: bytes,
) -> AudioAsset:
    settings = get_settings()
    if len(data) > settings.max_upload_bytes:
        raise PayloadTooLargeError(
            f"File exceeds MAX_UPLOAD_MB={settings.max_upload_mb}"
        )
    audio_id = uuid4()
    storage = get_storage(settings)
    stored = storage.write_original(audio_id, filename, data)
    asset = AudioAsset(
        id=audio_id,
        original_filename=filename,
        stored_filename=stored,
        mime_type=_mime(filename),
        status="uploaded",
    )
    session.add(asset)
    await EventService.append(
        session,
        event_type="audio_uploaded",
        entity_type="audio",
        entity_id=audio_id,
        payload={"filename": filename, "bytes": len(data)},
    )
    await session.commit()
    await session.refresh(asset)
    return asset


async def enqueue_analysis(
    session: AsyncSession,
    task_runner: TaskRunner,
    *,
    audio_id: UUID,
) -> Analysis:
    settings = get_settings()
    asset = await _get_asset(session, audio_id)
    analysis = Analysis(
        id=uuid4(),
        audio_id=asset.id,
        status="pending",
        analyzer_version=settings.analyzer_version,
    )
    asset.status = "analyzing"
    session.add(analysis)
    await session.commit()
    await session.refresh(analysis)
    await task_runner.enqueue(
        "analyze_audio",
        {"analysis_id": str(analysis.id)},
    )
    return analysis


async def upload_and_analyze(
    session: AsyncSession,
    task_runner: TaskRunner,
    *,
    filename: str,
    data: bytes,
) -> tuple[AudioAsset, Analysis]:
    asset = await create_asset_from_bytes(session, filename=filename, data=data)
    analysis = await enqueue_analysis(session, task_runner, audio_id=asset.id)
    return asset, analysis


async def get_audio_detail(
    session: AsyncSession, audio_id: UUID
) -> tuple[AudioAsset, Analysis | None]:
    asset = await _get_asset(session, audio_id)
    result = await session.execute(
        select(Analysis)
        .where(Analysis.audio_id == audio_id)
        .order_by(Analysis.created_at.desc())
        .limit(1)
    )
    return asset, result.scalar_one_or_none()


def execute_analysis(analysis_id: UUID) -> None:
    """Sync pipeline used by Celery and LocalTaskRunner."""
    settings = get_settings()
    storage = get_storage(settings)
    from app.database.session import sync_session_factory

    session = sync_session_factory()()
    try:
        analysis = session.get(Analysis, analysis_id)
        if analysis is None:
            raise AnalysisError(f"Analysis {analysis_id} not found")
        asset = session.get(AudioAsset, analysis.audio_id)
        if asset is None:
            raise AnalysisError(f"Audio asset {analysis.audio_id} not found")

        analysis.status = "running"
        session.commit()

        path = storage.original_path(asset.id, asset.stored_filename)
        loaded = load_audio(path)
        features = extract_features(loaded, analyzer_version=settings.analyzer_version)
        issues = detect_issues(features)
        payload = {
            "analysis_id": str(analysis.id),
            "audio_id": str(asset.id),
            "analyzer_version": settings.analyzer_version,
            "features": features.model_dump(),
            "issues": [issue.model_dump() for issue in issues],
        }
        storage.write_latest_analysis(asset.id, payload)

        analysis.status = "completed"
        analysis.features = features.model_dump()
        analysis.issues = [issue.model_dump() for issue in issues]
        analysis.error = None
        analysis.completed_at = datetime.now(UTC)
        analysis.analyzer_version = settings.analyzer_version
        asset.status = "ready"
        asset.duration_sec = features.duration_sec
        EventService.append_sync(
            session,
            event_type="analysis_completed",
            entity_type="audio",
            entity_id=asset.id,
            payload={
                "analysis_id": str(analysis.id),
                "analyzer_version": settings.analyzer_version,
            },
        )
        session.commit()
        logger.info("Analysis %s completed for audio %s", analysis.id, asset.id)
    except Exception as exc:
        logger.exception("Analysis %s failed", analysis_id)
        session.rollback()
        analysis = session.get(Analysis, analysis_id)
        if analysis is not None:
            analysis.status = "failed"
            analysis.error = str(exc)
            analysis.completed_at = datetime.now(UTC)
            asset = session.get(AudioAsset, analysis.audio_id)
            if asset is not None:
                asset.status = "failed"
                EventService.append_sync(
                    session,
                    event_type="analysis_failed",
                    entity_type="audio",
                    entity_id=asset.id,
                    payload={"analysis_id": str(analysis.id), "error": str(exc)},
                )
            session.commit()
        raise
    finally:
        session.close()
