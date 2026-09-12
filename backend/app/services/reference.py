from __future__ import annotations

import asyncio
import tempfile
from pathlib import Path
from uuid import UUID

from sqlalchemy.ext.asyncio import AsyncSession

from app.audio.dsp.features import extract_features
from app.audio.intelligence.reference import compare_features
from app.audio.loader import load_audio
from app.core.config import get_settings
from app.core.exceptions import NotFoundError, PayloadTooLargeError, UnsupportedMediaError
from app.models.audio_asset import AudioAsset
from app.schemas.audio import AudioFeatures
from app.schemas.reference import ReferenceReport
from app.services.recommendation import _resolve_analysis
from app.services.storage import ALLOWED_EXTENSIONS


def _extract_path(path: Path) -> AudioFeatures:
    return extract_features(load_audio(path))


async def compare_reference(
    session: AsyncSession,
    *,
    audio_id: UUID,
    filename: str,
    data: bytes,
) -> ReferenceReport:
    settings = get_settings()
    if len(data) > settings.max_upload_bytes:
        raise PayloadTooLargeError(
            f"File exceeds MAX_UPLOAD_MB={settings.max_upload_mb}"
        )
    ext = Path(filename).suffix.lstrip(".").lower()
    if ext not in ALLOWED_EXTENSIONS:
        raise UnsupportedMediaError(
            f"Unsupported audio format '.{ext or 'unknown'}'. Use wav, mp3, or flac."
        )

    analysis = await _resolve_analysis(session, audio_id=audio_id, analysis_id=None)
    if analysis.status != "completed" or not analysis.features:
        raise NotFoundError("Analysis is not completed")

    asset = await session.get(AudioAsset, audio_id)
    target_name = asset.original_filename if asset is not None else "track"
    target = AudioFeatures.model_validate(analysis.features)

    tmp_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(suffix=f".{ext}", delete=False) as tmp:
            tmp.write(data)
            tmp_path = Path(tmp.name)
        reference = await asyncio.to_thread(_extract_path, tmp_path)
    finally:
        if tmp_path is not None:
            tmp_path.unlink(missing_ok=True)

    return compare_features(
        target,
        reference,
        target_filename=target_name,
        reference_filename=filename,
    )
