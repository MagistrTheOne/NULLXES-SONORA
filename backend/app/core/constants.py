"""Cross-cutting constants that must stay stable across API, CLI, and workers."""

from typing import Final, Literal

ANALYZER_VERSION_DEFAULT: Final[str] = "SONORA_DSP_v0.1"

ALLOWED_AUDIO_EXTENSIONS: Final[frozenset[str]] = frozenset({"wav", "mp3", "flac"})
ALLOWED_AUDIO_MIME_TYPES: Final[frozenset[str]] = frozenset(
    {
        "audio/wav",
        "audio/x-wav",
        "audio/wave",
        "audio/mpeg",
        "audio/mp3",
        "audio/flac",
        "audio/x-flac",
        "application/octet-stream",
    }
)

OWNER_PROFILE_ID: Final[int] = 1

TASK_ANALYZE_AUDIO: Final[str] = "analyze_audio"

AnalysisStatus = Literal["pending", "running", "completed", "failed"]
AudioAssetStatus = Literal["uploaded", "analyzing", "ready", "failed"]
DecisionValue = Literal["accepted", "rejected", "modified"]
TaskRunnerName = Literal["celery", "local"]
LLMProviderName = Literal["openai", "anthropic", "google"]

EVENT_AUDIO_UPLOADED: Final[str] = "audio_uploaded"
EVENT_ANALYSIS_COMPLETED: Final[str] = "analysis_completed"
EVENT_ANALYSIS_FAILED: Final[str] = "analysis_failed"
EVENT_RECOMMENDATION_GENERATED: Final[str] = "recommendation_generated"
EVENT_DECISION_RECORDED: Final[str] = "decision_recorded"

ENTITY_AUDIO: Final[str] = "audio"
ENTITY_ANALYSIS: Final[str] = "analysis"
ENTITY_RECOMMENDATION: Final[str] = "recommendation"
ENTITY_DECISION: Final[str] = "decision"
ENTITY_OWNER: Final[str] = "owner"
