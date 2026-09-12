from uuid import UUID

from app.core.logging import configure_logging
from app.core.config import get_settings
from app.database.session import init_engines
from app.services.analysis import execute_analysis
from app.workers.celery import celery_app

configure_logging(get_settings())
init_engines()


@celery_app.task(name="analyze_audio")
def analyze_audio(analysis_id: str) -> str:
    execute_analysis(UUID(analysis_id))
    return analysis_id
