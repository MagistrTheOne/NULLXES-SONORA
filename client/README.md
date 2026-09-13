# SONORA Runtime (v1.0.3)

Intelligent producer layer. Native JUCE 8. No Python at runtime.

**FREE VST3:** live listen, understand, mix diagnostics, structure.  
**Premium:** SONI — voice + chat. She greets with character.

In the DAW, SONORA listens to playback. Do not load a file. Press Play.
After about 15 seconds: TRACK UNDERSTOOD. Session memory is stored in
`Documents/NULLXES/SONORA/session.sonora` and in the DAW plugin state.

Standalone can still open wav/mp3/flac.

```
LISTEN → UNDERSTAND → CREATE → SONI
```

Ctrl+L — SONORA LAB.  
Ctrl+J — SONI.

## Install VST3 (FL Studio)

FL Studio does not accept the CMake build folder as a VST3 path.
It only scans the system VST3 directory:

`C:\Program Files\Common Files\VST3\SONORA.vst3`

Release build copies the bundle there (`COPY_PLUGIN_AFTER_BUILD`).
Then: Options → Manage plugins → Find plugins. Do not add `client/build/...`.
