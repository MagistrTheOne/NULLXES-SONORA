FROM python:3.12-slim-bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
        libsndfile1 \
        ffmpeg \
        build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY pyproject.toml /app/pyproject.toml
COPY app /app/app
COPY alembic /app/alembic
COPY alembic.ini /app/alembic.ini
COPY tests /app/tests
COPY docker_entrypoint.py /app/docker_entrypoint.py

RUN pip install --no-cache-dir --upgrade pip \
    && pip install --no-cache-dir .[dev]

ENV PYTHONPATH=/app \
    PYTHONUNBUFFERED=1 \
    STORAGE_PATH=/data/storage

ENTRYPOINT ["python", "/app/docker_entrypoint.py"]
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
