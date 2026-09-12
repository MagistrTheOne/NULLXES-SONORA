from app.schemas.audio import AnalysisOut
from app.schemas.common import ORMModel
from app.schemas.recommendation import DecisionOut


class MemoryResponse(ORMModel):
    preferred_sound_profile: dict
    analyses: list[AnalysisOut]
    decisions: list[DecisionOut]
