from __future__ import annotations

import json
from pathlib import Path
from uuid import UUID

from app.core.config import Settings, get_settings
from app.core.exceptions import UnsupportedMediaError

ALLOWED_EXTENSIONS = {"wav", "mp3", "flac"}


class AudioStorage:
    """VST-compatible on-disk layout. Only this module knows the paths."""

    def __init__(self, root: Path | None = None) -> None:
        settings = get_settings()
        self.root = (root or settings.storage_path).resolve()

    def asset_dir(self, audio_id: UUID) -> Path:
        return self.root / "audio" / str(audio_id)

    def create_layout(self, audio_id: UUID) -> Path:
        directory = self.asset_dir(audio_id)
        (directory / "analysis").mkdir(parents=True, exist_ok=True)
        (directory / "waveform").mkdir(parents=True, exist_ok=True)
        (directory / "cache").mkdir(parents=True, exist_ok=True)
        return directory

    def original_path(self, audio_id: UUID, stored_filename: str) -> Path:
        return self.asset_dir(audio_id) / stored_filename

    def analysis_latest_path(self, audio_id: UUID) -> Path:
        return self.asset_dir(audio_id) / "analysis" / "latest.json"

    def extension_of(self, filename: str) -> str:
        ext = Path(filename).suffix.lstrip(".").lower()
        if ext not in ALLOWED_EXTENSIONS:
            raise UnsupportedMediaError(
                f"Unsupported audio format '.{ext or 'unknown'}'. "
                f"Allowed: {sorted(ALLOWED_EXTENSIONS)}"
            )
        return ext

    def write_original(self, audio_id: UUID, filename: str, data: bytes) -> str:
        ext = self.extension_of(filename)
        stored = f"original.{ext}"
        self.create_layout(audio_id)
        path = self.original_path(audio_id, stored)
        path.write_bytes(data)
        return stored

    def write_latest_analysis(self, audio_id: UUID, payload: dict) -> Path:
        self.create_layout(audio_id)
        path = self.analysis_latest_path(audio_id)
        path.write_text(
            json.dumps(payload, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )
        return path


def get_storage(settings: Settings | None = None) -> AudioStorage:
    settings = settings or get_settings()
    return AudioStorage(root=settings.storage_path)
