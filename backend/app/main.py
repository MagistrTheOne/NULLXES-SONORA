from __future__ import annotations

import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
from sqlalchemy import text

from app.api.v1.router import api_v1_router
from app.core.config import get_settings
from app.core.exceptions import SonoraError
from app.core.logging import configure_logging
from app.database.seed import seed_owner_profile
from app.database.session import async_session_factory, get_async_engine, init_engines
from app.schemas.common import HealthResponse

logger = logging.getLogger(__name__)


@asynccontextmanager
async def lifespan(_app: FastAPI):
    settings = get_settings()
    configure_logging(settings)
    settings.storage_path.mkdir(parents=True, exist_ok=True)
    init_engines(settings)
    factory = async_session_factory()
    async with factory() as session:
        await seed_owner_profile(session)
    logger.info(
        "SONORA started analyzer_version=%s task_runner=%s",
        settings.analyzer_version,
        settings.task_runner,
    )
    yield
    engine = get_async_engine()
    await engine.dispose()


app = FastAPI(
    title="NULLXES SONORA",
    version="0.1.0",
    description="Adaptive Sound Intelligence — Phase 1 backend",
    lifespan=lifespan,
)
app.include_router(api_v1_router)


@app.exception_handler(SonoraError)
async def sonora_error_handler(_request: Request, exc: SonoraError) -> JSONResponse:
    return JSONResponse(
        status_code=exc.status_code,
        content={"code": exc.code, "message": exc.message},
    )


@app.get("/health", response_model=HealthResponse)
async def health() -> HealthResponse:
    database = "ok"
    try:
        engine = get_async_engine()
        async with engine.connect() as connection:
            await connection.execute(text("SELECT 1"))
    except Exception as exc:
        logger.warning("Health check database failure: %s", exc)
        database = "error"
    return HealthResponse(
        status="ok" if database == "ok" else "degraded",
        database=database,
    )
