from collections.abc import AsyncIterator

from fastapi import Depends
from sqlalchemy.ext.asyncio import AsyncSession

from app.ai.base import LLMProvider
from app.ai.factory import get_llm_provider
from app.database.session import get_async_session
from app.services.task_runner import TaskRunner, get_task_runner


async def db_session() -> AsyncIterator[AsyncSession]:
    async for session in get_async_session():
        yield session


def task_runner() -> TaskRunner:
    return get_task_runner()


def llm_provider() -> LLMProvider:
    return get_llm_provider()


SessionDep = Depends(db_session)
TaskRunnerDep = Depends(task_runner)
LLMDep = Depends(llm_provider)
