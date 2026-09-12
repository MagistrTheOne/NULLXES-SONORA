# SONORA Architecture (Phase 1)

NULLXES SONORA is an internal Adaptive Sound Intelligence backend.
It is not MAGI, not a foundation model, and not a chatbot.

Phase 1 is a single-owner engineering layer for audio analysis,
structured mixing recommendations, and project memory.

## Entrances

- HTTP API (`/api/v1`) for a future VST, desktop app, and web UI
- CLI (`sonora analyze`, `sonora recommend`) for daily local use

Both call the same services. Services never import Celery directly.

## Layers

```
api / cli
    -> services (orchestration, memory, storage, events, TaskRunner)
        -> audio/dsp          (mathematics)
        -> audio/intelligence (engineering interpretation)
        -> ai                 (structured LLM recommendations only)
        -> database           (PostgreSQL)
```

DSP computes features. Intelligence labels issues. The LLM never
invents BPM, loudness, or spectrum — it only reasons over JSON.

## TaskRunner

`TaskRunner.enqueue(task_name, payload)` is the only async-job API.

- `CeleryTaskRunner` — Redis broker, production Compose
- `LocalTaskRunner` — in-process, CLI and tests

Ray or Kubernetes Jobs can replace the implementation later.

## Storage

```
{STORAGE_PATH}/audio/{audio_id}/
  original.{wav|mp3|flac}
  analysis/latest.json
  waveform/
  cache/
```

`services/storage.py` is the only module that knows this layout.

## Persistence

| Table              | Role                                      |
|--------------------|-------------------------------------------|
| owner_profiles     | Singleton owner taste / mix preferences   |
| audio_assets       | Uploaded files and status                 |
| analyses           | Versioned DSP results (`analyzer_version`)|
| recommendations    | Structured LLM output                     |
| owner_decisions    | Accepted / rejected / modified items      |
| system_events      | Append-only journal for a future VST poll |

## Uncertainty

- Loudness is `loudness_lufs_approx` (K-weighting, no EBU R128 gating).
- Key is `key_estimation {key, confidence, method}`. EDM/house chroma
  often drifts; low confidence is expected, not a failure.

## Out of scope (Phase 1)

Frontend, VST, model training, S3, JWT, SaaS users, websocket streams,
full EBU R128, waveform peak generation, PyTorch.
