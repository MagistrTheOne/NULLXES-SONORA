from __future__ import annotations

import asyncio
import logging
from typing import Protocol
from uuid import UUID, uuid4

logger = logging.getLogger(__name__)


class TaskRunner(Protocol):
    async def enqueue(self, task_name: str, payload: dict) -> str: ...


class LocalTaskRunner:
    """In-process runner for CLI and tests. Same pipeline as the worker."""

    async def enqueue(self, task_name: str, payload: dict) -> str:
        job_id = str(uuid4())
        if task_name == "analyze_audio":
            from app.services.analysis import execute_analysis

            analysis_id = UUID(payload["analysis_id"])
            await asyncio.to_thread(execute_analysis, analysis_id)
            return job_id
        raise ValueError(f"Unknown task: {task_name}")


class CeleryTaskRunner:
    async def enqueue(self, task_name: str, payload: dict) -> str:
        from app.workers.celery import celery_app

        result = celery_app.send_task(task_name, kwargs=payload)
        logger.info("Enqueued %s via Celery id=%s", task_name, result.id)
        return str(result.id)


def get_task_runner(name: str | None = None) -> TaskRunner:
    from app.core.config import get_settings

    runner = (name or get_settings().task_runner).lower()
    if runner == "celery":
        return CeleryTaskRunner()
    if runner == "local":
        return LocalTaskRunner()
    raise ValueError(f"Unsupported TASK_RUNNER: {runner}")
