from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest
import soundfile as sf
from sqlalchemy import create_engine, text

from app.core.config import get_settings


def write_stereo_tone(
    path: Path,
    *,
    sample_rate: int = 22050,
    seconds: float = 1.5,
    left_hz: float = 120.0,
    right_hz: float = 1000.0,
    amplitude: float = 0.25,
) -> Path:
    frames = int(sample_rate * seconds)
    t = np.linspace(0.0, seconds, frames, endpoint=False)
    left = amplitude * np.sin(2 * np.pi * left_hz * t)
    right = amplitude * np.sin(2 * np.pi * right_hz * t + 0.35)
    sf.write(path, np.stack([left, right], axis=1), sample_rate)
    return path


def write_mono_tone(
    path: Path,
    *,
    sample_rate: int = 22050,
    seconds: float = 1.0,
    hz: float = 440.0,
    amplitude: float = 0.3,
) -> Path:
    frames = int(sample_rate * seconds)
    t = np.linspace(0.0, seconds, frames, endpoint=False)
    sf.write(path, amplitude * np.sin(2 * np.pi * hz * t), sample_rate)
    return path


def write_clipped_stereo(path: Path, sample_rate: int = 22050) -> Path:
    frames = int(sample_rate * 0.8)
    t = np.linspace(0.0, 0.8, frames, endpoint=False)
    wave = np.clip(np.sin(2 * np.pi * 80 * t) * 4.0, -1.0, 1.0)
    sf.write(path, np.stack([wave, wave], axis=1), sample_rate)
    return path


@pytest.fixture
def stereo_wav(tmp_path: Path) -> Path:
    return write_stereo_tone(tmp_path / "stereo.wav")


@pytest.fixture
def mono_wav(tmp_path: Path) -> Path:
    return write_mono_tone(tmp_path / "mono.wav")


@pytest.fixture
def clipped_wav(tmp_path: Path) -> Path:
    return write_clipped_stereo(tmp_path / "clipped.wav")


def postgres_available() -> bool:
    try:
        settings = get_settings()
        engine = create_engine(settings.sync_database_url, pool_pre_ping=True)
        with engine.connect() as connection:
            connection.execute(text("SELECT 1"))
        engine.dispose()
        return True
    except Exception:
        return False


requires_postgres = pytest.mark.skipif(
    not postgres_available(),
    reason="PostgreSQL is required for API / pipeline tests",
)


class FakeLLMProvider:
    name = "fake"
    model = "test-model"

    async def complete_structured(self, messages: list, schema: dict) -> dict:
        return {
            "items": [
                {
                    "action": "Reduce bass area around 120Hz",
                    "target": "bass",
                    "frequency_hz": 120,
                    "rationale": "Low-end energy share is high relative to mids",
                    "priority": 2,
                },
                {
                    "action": "Increase vocal presence around 2-4kHz",
                    "target": "vocals",
                    "frequency_hz": 3000,
                    "rationale": "Owner profile prefers clean vocals",
                    "priority": 3,
                },
            ]
        }
