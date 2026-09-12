from datetime import datetime
from typing import Literal
from uuid import UUID

from pydantic import BaseModel, Field

from app.schemas.common import ORMModel

DynamicRangeLabel = Literal["low", "medium", "high"]


class KeyEstimation(BaseModel):
    key: str | None = None
    confidence: float = Field(ge=0.0, le=1.0)
    method: str = "chroma_cqt"


class SpectrumSummary(BaseModel):
    centroid_hz: float
    rolloff_hz: float
    band_energies: dict[str, float]


class FrequencyDistribution(BaseModel):
    sub: float
    low: float
    mid: float
    high: float
    air: float


class AudioFeatures(BaseModel):
    analyzer_version: str
    bpm: float
    duration_sec: float
    sample_rate: int
    channels: int
    rms: float
    peak: float
    loudness_lufs_approx: float
    dynamic_range_db: float
    dynamic_range_label: DynamicRangeLabel
    spectrum: SpectrumSummary
    frequency_distribution: FrequencyDistribution
    stereo_width: float = Field(ge=0.0, le=1.0)
    key_estimation: KeyEstimation


class Issue(BaseModel):
    type: Literal[
        "clipping",
        "muddy_low_end",
        "frequency_conflict",
        "narrow_stereo",
        "low_dynamic_range",
    ]
    severity: float = Field(ge=0.0, le=1.0)
    area: str | None = None
    detail: str | None = None


class AnalyzeByIdRequest(BaseModel):
    audio_id: UUID


class AudioAssetOut(ORMModel):
    id: UUID
    original_filename: str
    stored_filename: str
    mime_type: str
    status: str
    duration_sec: float | None
    created_at: datetime


class AnalysisOut(ORMModel):
    id: UUID
    audio_id: UUID
    status: str
    analyzer_version: str
    features: AudioFeatures | None = None
    issues: list[Issue] | None = None
    error: str | None = None
    created_at: datetime
    completed_at: datetime | None = None


class AudioDetailResponse(BaseModel):
    asset: AudioAssetOut
    analysis: AnalysisOut | None = None
