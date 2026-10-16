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

Completed on native GitHub runners:

- Windows 2022 / Visual Studio x64: standalone EXE and VST3 built; 2/2 CTest checks passed; Inno Setup EXE and portable ZIP generated.
- macOS 14: Intel + Apple Silicon Universal standalone app and VST3 built; 2/2 CTest checks passed; app/plugin ad-hoc signed; PKG generated.
- [Build run 38938345485](https://github.com/DeandreEsdaille/Abide/actions/runs/38938345485), source commit `03408254ed6c28f38e4da4413f6d8d8c6b34f59d`, completed successfully on both runners.
- Downloaded Windows and Mac artifact SHA-256 values matched their Actions digests; ZIP integrity checked. Windows standalone/VST3 inspected as x86-64 PE binaries. Mac PKG payload inspected: standalone app and VST3 both contain x86_64 and arm64 Mach-O slices, and install paths match Applications and Library/Audio/Plug-Ins/VST3.

Not completed:

- End-user package installation, trusted developer signing, Apple notarization, or native audio-device testing. The Mac binaries have ad-hoc signatures; installers are unsigned.
- Actual VST3 loading in Ableton, FL Studio, Cubase, Reaper or another DAW.
- Physical MIDI keyboard / device routing checks; browser MIDI permission flow.
- Long-duration, real-time CPU/latency tests, host automation sweeps, or DAW recall across platforms.

Native build and automated test results are verified above. Generated installers are prerelease artifacts, not evidence of real DAW or end-user installation testing. Source archives do not contain compiled installers; obtain them from the linked successful Actions run.
