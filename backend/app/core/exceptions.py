class SonoraError(Exception):
    status_code: int = 400
    code: str = "sonora_error"

    def __init__(self, message: str, *, code: str | None = None) -> None:
        super().__init__(message)
        self.message = message
        if code is not None:
            self.code = code


class NotFoundError(SonoraError):
    status_code = 404
    code = "not_found"


class ConflictError(SonoraError):
    status_code = 409
    code = "conflict"


class PayloadTooLargeError(SonoraError):
    status_code = 413
    code = "payload_too_large"


class UnsupportedMediaError(SonoraError):
    status_code = 415
    code = "unsupported_media"


class ConfigurationError(SonoraError):
    status_code = 500
    code = "configuration_error"


class LLMProviderError(SonoraError):
    status_code = 502
    code = "llm_provider_error"


class AnalysisError(SonoraError):
    status_code = 422
    code = "analysis_error"
