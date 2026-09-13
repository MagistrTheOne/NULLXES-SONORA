# SONORA Runtime (v1.0.1)

Standalone and VST3. Native JUCE 8, no WebView, no Python at runtime.

This target builds `SONORA.exe` and `SONORA.vst3`. DSP runs in-process.

## Build (Windows / Visual Studio 2022)

```bat
cd client
cmake -B build -G "Visual Studio 17 2022" -A x64 -DSONORA_JUCE_DIR=D:/NULLXES/_cache/JUCE-8.0.8
cmake --build build --config Release --target SONORA
cmake --build build --config Release --target SONORA_VST3_VST3
```

JUCE 8.0.8 is expected at `SONORA_JUCE_DIR` (or `client/third_party/JUCE`).

Binaries:

```
client\build\SONORA_artefacts\Release\SONORA.exe
client\build\SONORA_VST3_artefacts\Release\VST3\SONORA.vst3
```

## Workflow

`LISTEN` — what the track is.  
`IMPROVE` — mix health, problems, actions.  
`CREATE` — chord, bass, arrangement, MIDI.

Ctrl+L opens SONORA LAB. It is a tool, not a chat.

SONI is the premium voice + chat. She greets with character. Ctrl+J toggles her. Not a helpdesk.

The VST does not analyze in the audio callback. One engine session is shared across plugin windows. LISTEN records only from the instance you armed.
