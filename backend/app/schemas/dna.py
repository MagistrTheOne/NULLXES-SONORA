from __future__ import annotations

from typing import Literal

from pydantic import BaseModel, Field

RiskLevel = Literal["low", "medium", "high"]
ObjectType = Literal[
    "EQ_PROFILE",
    "MIDI_CLIP",
    "ARRANGEMENT_MAP",
    "REFERENCE_REPORT",
    "MASTER_CHAIN",
]
ObjectStatus = Literal["generated", "draft", "applied"]
SectionName = Literal["intro", "build", "drop", "break", "groove", "outro", "body"]
TranslationName = Literal["phone", "car", "club", "headphones"]


class IdentityKey(BaseModel):
    name: str | None = None
    confidence: float = Field(ge=0.0, le=1.0)


class TrackIdentity(BaseModel):
    tempo: float
    key: IdentityKey
    genre_profile: list[str] = Field(default_factory=list)


class EnergyMap(BaseModel):
    curve: list[float]
    hop_sec: float
    mean: float
    peak: float
    method: str = "rms_envelope"


class StructureSection(BaseModel):
    name: SectionName
    start: float
    end: float
    energy: float = Field(ge=0.0, le=1.0)
    bass_energy: float = Field(ge=0.0, le=1.0)
    transient_density: float = Field(ge=0.0, le=1.0)
    stereo_width: float = Field(ge=0.0, le=1.0)


class TrackStructure(BaseModel):
    sections: list[StructureSection]
    method: str = "novelty_peak + energy_label"


class MixAxis(BaseModel):
    sub: float | None = None
    low: float | None = None
    control: float | None = None
    value: float | None = None
    risk: RiskLevel
    why: str
    method: str


class MixCharacter(BaseModel):
    low_end: MixAxis
    brightness: MixAxis
    stereo: MixAxis
    dynamics: MixAxis


class TranslationTarget(BaseModel):
    name: TranslationName
    score: float = Field(ge=0.0, le=1.0)
    issue: str | None = None
    reason: str | None = None
    action: str | None = None


class TranslationLab(BaseModel):
    targets: list[TranslationTarget]
    method: str = "band_weight playback models"


class MaskingMap(BaseModel):
    roles: list[str]
    matrix: list[list[float]]
    method: str = "frequency_role_overlap"
    note: str = "Role proxies from frequency bands, not separated stems"


class SonoraObject(BaseModel):
    type: ObjectType
    input: str
    parameters: dict
    status: ObjectStatus = "generated"


class TrackDNA(BaseModel):
    identity: TrackIdentity
    energy: EnergyMap
    structure: TrackStructure
    mix_character: MixCharacter
    translation: TranslationLab
    masking: MaskingMap
    objects: list[SonoraObject] = Field(default_factory=list)
