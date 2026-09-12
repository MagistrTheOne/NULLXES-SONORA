from __future__ import annotations

import json

from google import genai
from google.genai import types

from app.ai.base import Message
from app.core.exceptions import LLMProviderError


class GoogleProvider:
    name = "google"

    def __init__(self, api_key: str, model: str) -> None:
        if not api_key:
            raise LLMProviderError("GOOGLE_API_KEY is not set")
        self.model = model
        self._client = genai.Client(api_key=api_key)

    async def complete_structured(
        self, messages: list[Message], schema: dict
    ) -> dict:
        contents: list[types.Content] = []
        system_parts: list[str] = []
        for message in messages:
            if message["role"] == "system":
                system_parts.append(message["content"])
                continue
            role = "user" if message["role"] == "user" else "model"
            contents.append(
                types.Content(
                    role=role,
                    parts=[types.Part.from_text(text=message["content"])],
                )
            )
        try:
            response = await self._client.aio.models.generate_content(
                model=self.model,
                contents=contents,
                config=types.GenerateContentConfig(
                    system_instruction="\n".join(system_parts) or None,
                    response_mime_type="application/json",
                    response_schema=schema,
                ),
            )
        except Exception as exc:
            raise LLMProviderError(f"Google request failed: {exc}") from exc

        text = getattr(response, "text", None)
        if not text:
            raise LLMProviderError("Google returned an empty response")
        try:
            return json.loads(text)
        except json.JSONDecodeError as exc:
            raise LLMProviderError("Google returned invalid JSON") from exc
