from app.ai.base import Message
from app.core.exceptions import ConfigurationError


class OffProvider:
    name = "off"
    model = "dsp-heuristic"

    def __init__(self) -> None:
        self.model = "dsp-heuristic"

    async def complete_structured(self, messages: list[Message], schema: dict) -> dict:
        raise ConfigurationError("LLM is off — DSP heuristics only")
