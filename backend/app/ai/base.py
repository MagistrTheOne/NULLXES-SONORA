from __future__ import annotations

from typing import Protocol, TypedDict


class Message(TypedDict):
    role: str
    content: str


class LLMProvider(Protocol):
    name: str
    model: str

    async def complete_structured(
        self,
        messages: list[Message],
        schema: dict,
    ) -> dict: ...
