# SONORA Runtime (Phase 2)

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

## UI (Phase 2.1)

Native JUCE, mock session on launch. No API yet.

Component names match a future shadcn Studio twin:

| JUCE | shadcn |
|---|---|
| MetricCard | Card |
| IssueRow | list item + Progress |
| ActionButton | Button outline/ghost |
| SpectrumView | Chart placeholder |
| InsightPanel | Card + structured fields |
| StatusRail | Footer + Badge |
| AssistantPanel | Textarea + ghost buttons |

Not yet: HTTP, real file load, OpenGL FFT, VST.

## Backend

Runtime will talk to the existing FastAPI app. Raise the backend separately:

```bat
cd backend
uvicorn app.main:app --reload
```

Default API: `http://127.0.0.1:8000`
