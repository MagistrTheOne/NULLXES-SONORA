from __future__ import annotations

from anthropic import AsyncAnthropic

from app.ai.base import Message
from app.core.exceptions import LLMProviderError


class AnthropicProvider:
    name = "anthropic"

    def __init__(self, api_key: str, model: str) -> None:
        if not api_key:
            raise LLMProviderError("ANTHROPIC_API_KEY is not set")
        self.model = model
        self._client = AsyncAnthropic(api_key=api_key)

    async def complete_structured(
        self, messages: list[Message], schema: dict
    ) -> dict:
        system = " ".join(
            message["content"] for message in messages if message["role"] == "system"
        )
        user_messages = [
            {"role": message["role"], "content": message["content"]}
            for message in messages
            if message["role"] != "system"
        ]
        try:
            response = await self._client.messages.create(
                model=self.model,
                max_tokens=2048,
                system=system,
                messages=user_messages,  # type: ignore[arg-type]
                tools=[
                    {
                        "name": "sonora_recommendations",
                        "description": "Structured mixing recommendations for SONORA",
                        "input_schema": schema,
                    }
                ],
                tool_choice={"type": "tool", "name": "sonora_recommendations"},
            )
        except Exception as exc:
            raise LLMProviderError(f"Anthropic request failed: {exc}") from exc

        for block in response.content:
            if getattr(block, "type", None) == "tool_use":
                data = getattr(block, "input", None)
                if isinstance(data, dict):
                    return data
        raise LLMProviderError("Anthropic returned no structured tool payload")
