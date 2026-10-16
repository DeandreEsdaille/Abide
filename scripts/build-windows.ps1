$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
cmake -S . -B build-windows -A x64
if ($LASTEXITCODE) { throw 'Configure failed' }
cmake --build build-windows --config Release --parallel 2
if ($LASTEXITCODE) { throw 'Build failed' }
ctest --test-dir build-windows -C Release --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
New-Item -ItemType Directory -Force dist/Windows | Out-Null
Copy-Item 'build-windows/Abide_artefacts/Release/Standalone/Abide.exe' dist/Windows/
Copy-Item 'build-windows/Abide_artefacts/Release/VST3/Abide.vst3' dist/Windows/ -Recurse -Force
Copy-Item scripts/Install-Windows.ps1 dist/Windows/
Copy-Item scripts/Abide.iss dist/Windows/
if (Get-Command ISCC.exe -ErrorAction SilentlyContinue) { ISCC.exe dist/Windows/Abide.iss; if ($LASTEXITCODE) { throw 'Installer failed' } }
Compress-Archive -Path dist/Windows/* -DestinationPath dist/Abide-0.2.0-Windows-x64.zip -Force
