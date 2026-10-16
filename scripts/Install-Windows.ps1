# Run as administrator from the extracted Windows binary package.
#Requires -RunAsAdministrator
$ErrorActionPreference='Stop'
$target=Join-Path $env:ProgramFiles 'Gimme That Juice/Abide'
$plugins=Join-Path $env:CommonProgramFiles 'VST3'
New-Item -ItemType Directory -Force $target,$plugins | Out-Null
Copy-Item (Join-Path $PSScriptRoot 'Abide.exe') $target -Force
Copy-Item (Join-Path $PSScriptRoot 'Abide.vst3') $plugins -Recurse -Force
Write-Host 'Abide installed. Launch Abide.exe from Program Files/Gimme That Juice/Abide. Rescan VST3 in your DAW.'
