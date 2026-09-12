# SONORA Runtime (Phase 2)

Standalone desktop shell. Native JUCE 8, no WebView, no VST yet.

This target is `SONORA.exe`. The future VST lives in `../plugin/`.

## Build (Windows / Visual Studio 2022)

```bat
cd client
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

First configure clones JUCE 8 via FetchContent.

Binary:

```
client\build\SONORA_artefacts\Release\SONORA.exe
```

## What this scaffold is

- Window + theme tokens
- Empty dashboard (NULLXES / SONORA)
- `AppState` + `BackendStatus`
- Model types (`AudioAnalysis`, `Issue`, `Insight`)

Not yet: HTTP, file load, analysis panels, OpenGL spectrum.

## Backend

Runtime will talk to the existing FastAPI app. Raise the backend separately:

```bat
cd backend
uvicorn app.main:app --reload
```

Default API: `http://127.0.0.1:8000`
