from __future__ import annotations

import json

from openai import AsyncOpenAI

from app.ai.base import Message
from app.core.exceptions import LLMProviderError


class OpenAIProvider:
    name = "openai"

    def __init__(self, api_key: str, model: str) -> None:
        if not api_key:
            raise LLMProviderError("OPENAI_API_KEY is not set")
        self.model = model
        self._client = AsyncOpenAI(api_key=api_key)

    async def complete_structured(
        self, messages: list[Message], schema: dict
    ) -> dict:
        try:
            response = await self._client.chat.completions.create(
                model=self.model,
                messages=messages,  # type: ignore[arg-type]
                response_format={
                    "type": "json_schema",
                    "json_schema": {
                        "name": "sonora_recommendations",
                        "strict": True,
                        "schema": schema,
                    },
                },
            )
        except Exception as exc:
            raise LLMProviderError(f"OpenAI request failed: {exc}") from exc

        content = response.choices[0].message.content
        if not content:
            raise LLMProviderError("OpenAI returned an empty response")
        try:
            return json.loads(content)
        except json.JSONDecodeError as exc:
            raise LLMProviderError("OpenAI returned invalid JSON") from exc
