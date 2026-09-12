# SONORA Runtime (v0.3)

Standalone desktop shell. Native JUCE 8, no WebView, no VST yet.

This target is `SONORA.exe`. The future VST lives in `../plugin/`.

## Build (Windows / Visual Studio 2022)

```bat
cd client
cmake -B build -G "Visual Studio 17 2022" -A x64 -DSONORA_JUCE_DIR=D:/NULLXES/_cache/JUCE-8.0.8
cmake --build build --config Release
```

JUCE 8.0.8 is expected at `SONORA_JUCE_DIR` (or `client/third_party/JUCE`).
A workspace path with spaces breaks FetchContent on Windows — clone JUCE to a path without spaces:

```bat
git clone --depth 1 --branch 8.0.8 https://github.com/juce-framework/JUCE.git D:\NULLXES\_cache\JUCE-8.0.8
```

Binary:

```
client\build\SONORA_artefacts\Release\SONORA.exe
```

## Workflow

UI lives off `AppState` / `AnalysisState`:

`Empty → Loading → Analyzing → Complete | Failed`

```
LOAD TRACK → ANALYZING → TRACK INTELLIGENCE → ENGINEERING REPORT → ACTION
```

Empty shows NO TRACK. Analyzing shows DSP progress. Complete builds a Track DNA model: identity, arrangement, energy, mix character, translation. Canvas nodes open the inspector. CREATE still writes objects. No LLM in this layer.

## Backend

Runtime talks to the FastAPI brain. Raise it separately (do not start Docker from here unless you intend to):

```bat
cd backend
uvicorn app.main:app --reload
```

Default API: `http://127.0.0.1:8000`

Used by the client:

- `GET /health`
- `POST /api/v1/audio/analyze` (multipart file)
- `GET /api/v1/audio/{id}`
- `POST /api/v1/recommendation/generate`
- `POST /api/v1/generate/harmony`
- `GET /api/v1/profile`

Harmony is MIDI-first JSON (`key`, `bars`, `chords`). No audio generation.
