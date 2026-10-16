# Abide — Gimme That Juice

Original atmospheric worship drone instrument, version 0.1.0 prototype.

## What is in this package

- Native C++ instrument targeting macOS Universal (Apple Silicon + Intel) and Windows x64, VST3 and standalone.
- Generated mountain-and-lake photographic artwork embedded into the native interface.
- Browser audio preview: open Preview.html in Chrome or Edge, then click Enable Audio and a note. It needs no installation or account. It is a separate Web Audio implementation for trying the concept, not the VST3 binary; its sound and reverb differ from the native engine.
- Build scripts, a GitHub Actions build workflow, automated engine and processor checks.
- Render a native audio demo after building with: AbideSmoke --render Abide-Audio-Demo.wav.

Native macOS Universal and Windows x64 builds passed on 16 October 2026, including both automated engine and processor tests. Download the VST3/standalone installers from the [successful build run](https://github.com/DeandreEsdaille/Abide/actions/runs/38938345485). This is a prerelease: installation on end-user machines, physical audio/MIDI devices, and real DAW compatibility have not yet been tested.

## Play

Choose C through B to start a drone in the C3–B3 register. MIDI note-on selects the root in its played register (clamped to C1–C6). Only the latest root is active; previous roots release smoothly. Latch is enabled by default: MIDI note-off leaves the drone playing. Disable Latch for MIDI note-held playing. The native on-screen buttons always start a latched drone; Fade Out stops it. No audio plays on startup.

Fade Out releases voices and lets delay/reverb decay. Panic clears voices and both effects immediately. The standalone JUCE Options/Audio settings menu selects output device, sample rate, and active MIDI inputs. Start with low output volume. In a DAW, load Abide as a stereo instrument and send MIDI to it.

The browser preview also supports computer keys A W S E D F T G Y H U J and Space to fade. Chrome/Edge Web MIDI depends on browser permissions, secure-context rules, and device availability. Audio buttons work without MIDI. No cloud sound downloads are used.

## Parameters

| Parameter | Range | Purpose |
| --- | --- | --- |
| Volume | 0–1 | Smoothed master level, including effect tails in native plugin |
| Tone | 0–1 | Low-pass cutoff, roughly 250 Hz–10 kHz |
| Movement | 0–1 | Slow breathing modulation |
| Width | 0–1.5 | Stereo spread; 0 is mono before reverb |
| Attack | 0.05–10 seconds | Exponential swell time constant in native engine; onset is gradual |
| Release | 0.1–20 seconds | Exponential release time constant; effect tails continue afterward |
| Reverb | 0–1 | Reverb amount |
| Room Size | 0–1 | Native reverb room size; impulse length in preview |
| Delay | 0–1 | Echo amount |
| Delay Time | 40–1800 ms | Smoothed echo time; manual milliseconds, no host tempo sync in this version |
| Feedback | 0–0.85 | Echo regeneration |
| Texture | Velvet / Radiance / Halo | Alternate harmonic colours |
| Latch | On / Off | Hold MIDI-triggered drone after note release |

Four starting presets: Still Waters, Prayer Room, Open Heaven, Morning Light. Parameters can be automated and are stored in DAW project state. The plugin does not currently save standalone sessions, import samples, include a piano instrument, expose separate MIDI zones, or implement tempo-sync. Texture changes apply immediately in native engine and on the next note in the browser preview. This project uses original synthesized tones, not any Presence samples, artwork, or code.

## Build macOS

Install Xcode Command Line Tools, Git, and CMake 3.22 or newer. With this folder extracted:

```bash
bash scripts/build-mac.sh
```

Creates `dist/Abide-0.1.0-macOS-Universal.pkg`, containing `/Applications/Abide.app` and `/Library/Audio/Plug-Ins/VST3/Abide.vst3`. Targets macOS 11 or newer. The package is unsigned and binaries are only ad-hoc signed; a distributable trusted release requires Apple Developer signing and notarization. This script does not disable Gatekeeper or alter security settings.

## Build Windows

Install Visual Studio 2022 with Desktop development with C++, Windows SDK, Git, and CMake. Optionally install Inno Setup 6 and put ISCC.exe on PATH for a setup EXE. Open a PowerShell terminal in this folder:

```powershell
./scripts/build-windows.ps1
```

Creates `dist/Abide-0.1.0-Windows-x64.zip`; with Inno Setup also creates a Setup.exe. The ZIP includes Abide.exe, the entire Abide.vst3 bundle and an administrator installation script. Install the full VST3 bundle, not just its inner binary. Windows executables are unsigned. Requires the current Microsoft Visual C++ 2015–2022 x64 Redistributable on target machines. Native Windows audio uses JUCE's default device backends; no proprietary ASIO SDK is bundled.

## GitHub builds

Put the contents of this folder at the root of a Git repository; include `.github/workflows/build.yml` and the artwork. Push or run Build Abide manually in Actions. The macOS and Windows jobs produce downloadable artifacts. The reference macOS and Windows jobs both passed in [run 38938345485](https://github.com/DeandreEsdaille/Abide/actions/runs/38938345485). Downloading source does not run a build automatically.

JUCE 8.0.6 is pinned to commit 51a8a6d7aeae7326956d747737ccf1575e61e209. CMake downloads it on the first build, so internet access is required. Set `-DJUCE_PATH=/absolute/path/to/JUCE` to use a local copy. No JUCE checkout is included in this ZIP.

## Validation

See VALIDATION.md for checks actually performed and remaining platform checks. Before public release, verify the signed builds in actual DAWs on Windows, Intel Mac, and Apple Silicon, test MIDI devices and audio device changes, and check long-session stability and CPU use.

## Code and dependency licensing

Original source in this package is provided under the MIT license in LICENSE.txt. JUCE and its bundled dependencies retain their own licenses. JUCE 8 is dual licensed under AGPLv3 or a commercial JUCE license; choose and comply with the appropriate route before distribution. See https://github.com/juce-framework/JUCE/blob/8.0.6/LICENSE.md . MIT licensing of the original code does not replace JUCE's licensing requirements.

## Artwork

Assets/atmosphere.jpg was created with the built-in image generation tool for this project. Final prompt: cinematic natural landscape photograph of layered misty mountains above a dark lake at blue hour, warm dawn light through clouds, lavender and muted teal, deep navy foreground, wide quiet lower area for controls; peaceful and spacious; no text, logos, UI, people, or instruments. The original full prompt is in Assets/artwork-prompt.txt.
