from __future__ import annotations

import argparse
import asyncio
import json
import sys
from pathlib import Path

from app.core.config import get_settings
from app.core.logging import configure_logging
from app.database.seed import seed_owner_profile
from app.database.session import async_session_factory, init_engines
from app.services.analysis import create_asset_from_bytes, enqueue_analysis
from app.services.recommendation import generate_recommendations
from app.services.storage import get_storage
from app.services.task_runner import LocalTaskRunner


def _boot() -> None:
    settings = get_settings()
    configure_logging(settings)
    settings.storage_path.mkdir(parents=True, exist_ok=True)
    init_engines(settings)


async def _analyze(path: Path) -> dict:
    _boot()
    if not path.exists():
        raise FileNotFoundError(path)
    data = path.read_bytes()
    factory = async_session_factory()
    runner = LocalTaskRunner()
    async with factory() as session:
        await seed_owner_profile(session)
        asset = await create_asset_from_bytes(session, filename=path.name, data=data)
        analysis = await enqueue_analysis(session, runner, audio_id=asset.id)
        storage = get_storage()
        latest = storage.analysis_latest_path(asset.id)
        payload = json.loads(latest.read_text(encoding="utf-8"))
        payload["status"] = analysis.status
        return payload


async def _recommend(path: Path) -> dict:
    _boot()
    payload = json.loads(path.read_text(encoding="utf-8"))
    factory = async_session_factory()
    async with factory() as session:
        await seed_owner_profile(session)
        result = await generate_recommendations(
            session,
            analysis_override=payload,
            persist=False,
        )
    return result.model_dump()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="sonora",
        description="NULLXES SONORA — local audio intelligence CLI",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    analyze = sub.add_parser("analyze", help="Run DSP analysis on a wav/mp3/flac file")
    analyze.add_argument("path", type=Path)

    recommend = sub.add_parser(
        "recommend", help="Generate structured recommendations from analysis JSON"
    )
    recommend.add_argument("analysis_json", type=Path)

    args = parser.parse_args(argv)
    try:
        if args.command == "analyze":
            result = asyncio.run(_analyze(args.path))
        else:
            result = asyncio.run(_recommend(args.analysis_json))
    except Exception as exc:
        print(f"sonora: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(result, indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
