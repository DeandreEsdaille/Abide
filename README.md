# Abide — Gimme That Juice

Original atmospheric worship drone instrument, version 0.4.0 prerelease.

## New in 0.4

- Original violet orbital interface with a cached particle constellation scene.
- Layered chromatic note keys, four illuminated preset pads and sculpted volume/tone knobs.
- Luminous horizontal reverb and delay faders, plus compact controls for the remaining parameters.
- Native sample selection and import controls remain accessible beside the keyboard.

## New in 0.3

- Premium hardware-inspired native editor: sculpt, space and master fader banks.
- Machined metal fader caps, recessed illuminated rails, engraved scales and raised key buttons.
- Atmospheric image header, brushed dark panels and warm brass accents.
- All existing sample, MIDI, automation, preset and effect parameters retained.

## New in 0.2

- **Drone Sound** menu with two bundled CC0 recordings: Heavenly Choir (isaiah658 / OpenGameArt) and Cloud Drift (indigo.cloud / Freesound), plus the original synthesizer.
- **Import Drones** for up to eight WAV, MP3, FLAC or AIFF recordings; named sound selection and Sample Root control.
- Sample playback with looping, transposition, smooth sound changes and the existing effects. Major/minor quality is preserved from the recording.
- Private local cache and sample-library references saved in plugin/app state. No audio uploads or online service is required for playback.
- Filename root guesses for note prefixes; set the root manually for other names. Imports use up to 60 seconds at 32 kHz stereo; long files start 5 seconds in. Pitch transposition also changes playback speed. Missing or invalid files are reported.

See [SAMPLE-CREDITS.md](SAMPLE-CREDITS.md) for creators, licenses and preparation details. Sample selection/importing are native features; the browser preview remains a separate synthesizer demo.

## What is in this package

- Native C++ instrument targeting macOS Universal (Apple Silicon + Intel) and Windows x64, VST3 and standalone.
- Generated mountain-and-lake photographic artwork embedded into the native interface.
- Browser audio preview: open Preview.html in Chrome or Edge, then click Enable Audio and a note. It needs no installation or account. It is a separate Web Audio implementation for trying the concept, not the VST3 binary; its sound and reverb differ from the native engine.
- Build scripts, a GitHub Actions build workflow, automated engine and processor checks.
- Render a native audio demo after building with: AbideSmoke --render Abide-Audio-Demo.wav.

Native macOS Universal and Windows x64 builds passed on 16 October 2026, including all three automated engine, processor and sample tests. Download the VST3/standalone installers from the [successful build run](https://github.com/DeandreEsdaille/Abide/actions/runs/38949918514). This is a prerelease: installation on end-user machines, physical audio/MIDI devices, and real DAW compatibility have not yet been tested.

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

Four starting presets: Still Waters, Prayer Room, Open Heaven, Morning Light. Parameters and sound selection can be automated and are stored in DAW project state. Imported sample paths/names/root notes are also saved; the audio itself stays in a local cache. The plugin does not include a piano instrument, separate MIDI zones, time-stretching or host tempo-sync. The native sample library contains 2 factory sounds and 8 imported slots per instance. Open a new instance for additional imports when the slots or the 256 MiB sample-memory budget are full. Texture changes apply immediately in native engine and on the next note in the browser preview. Original synthesized tones and the two credited CC0 recordings are used; no Presence samples, artwork or code are included.

## Build macOS

Install Xcode Command Line Tools, Git, and CMake 3.22 or newer. With this folder extracted:

```bash
bash scripts/build-mac.sh
```

Creates `dist/Abide-0.4.0-macOS-Universal.pkg`, containing `/Applications/Abide.app` and `/Library/Audio/Plug-Ins/VST3/Abide.vst3`. Targets macOS 11 or newer. The package is unsigned and binaries are only ad-hoc signed; a distributable trusted release requires Apple Developer signing and notarization. This script does not disable Gatekeeper or alter security settings.

## Build Windows

Install Visual Studio 2022 with Desktop development with C++, Windows SDK, Git, and CMake. Optionally install Inno Setup 6 and put ISCC.exe on PATH for a setup EXE. Open a PowerShell terminal in this folder:

```powershell
./scripts/build-windows.ps1
```

Creates `dist/Abide-0.4.0-Windows-x64.zip`; with Inno Setup also creates a Setup.exe. The ZIP includes Abide.exe, the entire Abide.vst3 bundle and an administrator installation script. Install the full VST3 bundle, not just its inner binary. Windows executables are unsigned. Requires the current Microsoft Visual C++ 2015–2022 x64 Redistributable on target machines. Native Windows audio uses JUCE's default device backends; no proprietary ASIO SDK is bundled.

## GitHub builds

Put the contents of this folder at the root of a Git repository; include `.github/workflows/build.yml` and the artwork. Push or run Build Abide manually in Actions. The macOS and Windows jobs produce downloadable artifacts. The reference macOS and Windows jobs both passed in [run 38949918514](https://github.com/DeandreEsdaille/Abide/actions/runs/38949918514). Downloading source does not run a build automatically.

JUCE 8.0.6 is pinned to commit 51a8a6d7aeae7326956d747737ccf1575e61e209. CMake downloads it on the first build, so internet access is required. Set `-DJUCE_PATH=/absolute/path/to/JUCE` to use a local copy. No JUCE checkout is included in this ZIP.

## Validation

See VALIDATION.md for checks actually performed and remaining platform checks. Before public release, verify the signed builds in actual DAWs on Windows, Intel Mac, and Apple Silicon, test MIDI devices and audio device changes, and check long-session stability and CPU use.

## Code and dependency licensing

Original source in this package is provided under the MIT license in LICENSE.txt. JUCE and its bundled dependencies retain their own licenses. JUCE 8 is dual licensed under AGPLv3 or a commercial JUCE license; choose and comply with the appropriate route before distribution. See https://github.com/juce-framework/JUCE/blob/8.0.6/LICENSE.md . MIT licensing of the original code does not replace JUCE's licensing requirements.

## Artwork

Assets/atmosphere.jpg was created with the built-in image generation tool for this project. Final prompt: cinematic natural landscape photograph of layered misty mountains above a dark lake at blue hour, warm dawn light through clouds, lavender and muted teal, deep navy foreground, wide quiet lower area for controls; peaceful and spacious; no text, logos, UI, people, or instruments. The original full prompt is in Assets/artwork-prompt.txt.
