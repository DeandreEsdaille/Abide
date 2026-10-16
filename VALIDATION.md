# Validation — 16 October 2026

Completed on Linux x86_64:

- JUCE 8.0.6 / GCC / CMake Release build of native standalone and VST3 succeeded.
- Native VST3 module information generated successfully.
- Engine tests passed at 44.1, 48 and 96 kHz: startup silence, generated audio, finite samples, release to silence, 100 rapid root changes, maximum feedback stability and immediate engine reset.
- Native processor smoke test passed: MIDI note input at a sample offset, stereo audio through the effect chain, bounded output, project-state serialization/restoration and MIDI all-sound-off silence.
- CTest: 2 of 2 tests passed after final native changes.
- Native editor rendered to an offscreen image and visually inspected; units and numeric formatting corrected and checked again.
- A 24-second stereo PCM WAV rendered from the native processor with reverb/delay; peak approximately 0.352 FS, RMS approximately 0.078 FS. This is an offline render; no physical audio device was available.
- Playwright with Chromium tested the Web Audio preview: audio activation, nonzero audio samples, note selection, reverb/delay input, preset switching, fade, panic, and momentary mouse playing with Latch disabled. No JavaScript runtime errors in these tests. Browser screenshot inspected for layout.
- Source archive checked for included assets, build workflow, source, documentation, and playable preview.

Not completed:

- Windows or macOS compilation, package installation, signing, notarization, or native audio-device testing.
- Actual VST3 loading in Ableton, FL Studio, Cubase, Reaper or another DAW.
- Physical MIDI keyboard / device routing checks; browser MIDI permission flow.
- Long-duration, real-time CPU/latency tests, host automation sweeps, or DAW recall across platforms.

The included workflow and installer scripts are build configuration, not evidence that those platform builds passed. No macOS/Windows binaries are included or claimed as ready.
