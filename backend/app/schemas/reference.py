from __future__ import annotations

from pydantic import BaseModel


class ReferenceGap(BaseModel):
    loudness_lufs: float
    low_end: float
    stereo: float
    brightness: float
    method: str = "dsp_feature_delta"


class ReferenceReport(BaseModel):
    target_filename: str
    reference_filename: str
    gap: ReferenceGap
    notes: list[str]
    object: dict
