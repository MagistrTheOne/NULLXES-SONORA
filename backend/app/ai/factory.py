from __future__ import annotations

from app.ai.base import LLMProvider
from app.ai.providers.anthropic import AnthropicProvider
from app.ai.providers.google import GoogleProvider
from app.ai.providers.off import OffProvider
from app.ai.providers.openai import OpenAIProvider
from app.core.config import Settings, get_settings
from app.core.exceptions import ConfigurationError


def get_llm_provider(
    settings: Settings | None = None,
    override: LLMProvider | None = None,
) -> LLMProvider:
    if override is not None:
        return override
    settings = settings or get_settings()
    name = settings.llm_provider.lower()
    if name in {"off", "none", "disabled"}:
        return OffProvider()
    if name == "openai":
        return OpenAIProvider(settings.openai_api_key, settings.openai_model)
    if name == "anthropic":
        return AnthropicProvider(settings.anthropic_api_key, settings.anthropic_model)
    if name == "google":
        return GoogleProvider(settings.google_api_key, settings.google_model)
    raise ConfigurationError(f"Unknown LLM_PROVIDER: {settings.llm_provider}")
