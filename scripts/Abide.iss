[Setup]
AppId=com.gimmethatjuice.abide
AppName=Abide
AppVersion=0.2.0
AppPublisher=Gimme That Juice
DefaultDirName={autopf}\Gimme That Juice\Abide
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..
OutputBaseFilename=Abide-0.2.0-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
[Files]
Source: "Abide.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "Abide.vst3\*"; DestDir: "{commoncf64}\VST3\Abide.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{autoprograms}\Abide"; Filename: "{app}\Abide.exe"
