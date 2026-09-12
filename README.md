# NULLXES SONORA

Adaptive Sound Intelligence — Phase 1 backend.

SONORA is an engineering assistant for audio analysis, structured mixing
recommendations, and project memory. It is not MAGI, not a foundation model,
and not a chatbot.

Phase 1 is a single-owner internal tool: FastAPI + PostgreSQL + Redis + Celery
behind a `TaskRunner`, real DSP (`librosa` / `scipy`), and a swappable LLM
provider. No frontend. No VST. No model training.

## Architecture

```
VST / desktop / web (later)     CLI (now)
              \                 /
               services
          /      |       |      \
       DSP   intelligence  AI   TaskRunner
                                celery | local
```

Details: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

## Quick start (Docker)

```bash
copy .env.example .env
docker compose up --build
```

API: http://localhost:8000  
Docs: http://localhost:8000/docs  
Health: http://localhost:8000/health

Put an LLM key in `.env` before calling `/recommendation/generate`.
Analysis works without an LLM key.

## HTTP v1

```bash
curl -X POST http://localhost:8000/api/v1/audio/analyze ^
  -F "file=@track.wav"

curl http://localhost:8000/api/v1/audio/<audio_id>

curl -X POST http://localhost:8000/api/v1/recommendation/generate ^
  -H "Content-Type: application/json" ^
  -d "{\"audio_id\":\"<audio_id>\"}"

curl http://localhost:8000/api/v1/profile
curl http://localhost:8000/api/v1/memory
curl http://localhost:8000/api/v1/events
```

`POST /api/v1/audio/analyze` also accepts JSON `{"audio_id":"..."}` after upload.
Analysis is async: the call returns `202` and `GET /audio/{id}` carries the result.
With `TASK_RUNNER=local` the worker runs in-process (CLI and tests).

## CLI

```bash
cd backend
pip install -e .[dev]
alembic upgrade head
sonora analyze path\to\track.wav
sonora recommend path\to\analysis.json
```

CLI always uses `LocalTaskRunner` and writes:

```
storage/audio/{audio_id}/
  original.wav
  analysis/latest.json
  waveform/
  cache/
```

## Local API without Compose worker

```bash
cd backend
set TASK_RUNNER=local
alembic upgrade head
uvicorn app.main:app --reload
```

## Tests

Unit DSP tests do not need PostgreSQL. API and pipeline tests do.

```bash
docker compose up -d postgres redis
cd backend
pytest
```

## Environment

See [.env.example](.env.example). Important knobs:

| Variable | Meaning |
|---|---|
| `TASK_RUNNER` | `celery` or `local` |
| `LLM_PROVIDER` | `openai` / `anthropic` / `google` |
| `ANALYZER_VERSION` | stored on every analysis (`SONORA_DSP_v0.1`) |
| `STORAGE_PATH` | VST-compatible audio layout |

Loudness is `loudness_lufs_approx` (K-weighting, no EBU R128 claim).
Key is `key_estimation {key, confidence, method}` — EDM/house chroma can drift.
