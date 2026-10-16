#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build-mac -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build-mac --config Release --parallel 2
ctest --test-dir build-mac -C Release --output-on-failure
mkdir -p dist/mac-root/Applications dist/mac-root/Library/Audio/Plug-Ins/VST3
 ditto build-mac/Abide_artefacts/Release/Standalone/Abide.app dist/mac-root/Applications/Abide.app
 ditto build-mac/Abide_artefacts/Release/VST3/Abide.vst3 dist/mac-root/Library/Audio/Plug-Ins/VST3/Abide.vst3
codesign --force --deep --sign - dist/mac-root/Applications/Abide.app
codesign --force --deep --sign - dist/mac-root/Library/Audio/Plug-Ins/VST3/Abide.vst3
pkgbuild --root dist/mac-root --identifier com.gimmethatjuice.abide --version 0.3.0 --install-location / dist/Abide-0.3.0-macOS-Universal.pkg
