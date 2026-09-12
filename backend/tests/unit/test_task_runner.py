import pytest

from app.services.task_runner import LocalTaskRunner


@pytest.mark.asyncio
async def test_local_runner_rejects_unknown_task() -> None:
    runner = LocalTaskRunner()
    with pytest.raises(ValueError, match="Unknown task"):
        await runner.enqueue("not_a_task", {})
