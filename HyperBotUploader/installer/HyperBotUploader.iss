#define MyAppName "HyperBotUploader"
#define MyAppVersion "1.0.0"
#define MyAppExeName "HyperBotUploader.exe"

[Setup]
AppId={{C82A9B26-67F8-4B98-AD37-16B3E9C58A71}
AppName={#MyAppName}
AppVersion={#MyAppVersion}

DefaultDirName={autopf}\HyperBotUploader
DefaultGroupName=HyperBotUploader

OutputDir=output
OutputBaseFilename=HyperBotUploader-Setup

Compression=lzma2
SolidCompression=yes

PrivilegesRequired=admin

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

UninstallDisplayName=HyperBotUploader
UninstallDisplayIcon={app}\HyperBotUploader.exe


[Dirs]

; PROS requires this directory
Name: "{app}\tools\pros-toolchain\tmp"


[Files]

; =========================================================
; HyperBotUploader + Qt
; =========================================================

Source: "..\deploy\*"; \
    DestDir: "{app}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs


; =========================================================
; PROS CLI
; =========================================================

Source: "..\third_party\pros\*"; \
    DestDir: "{app}\tools\pros"; \
    Flags: ignoreversion recursesubdirs createallsubdirs


; =========================================================
; PROS ARM toolchain
; =========================================================

Source: "..\third_party\pros-toolchain\*"; \
    DestDir: "{app}\tools\pros-toolchain"; \
    Flags: ignoreversion recursesubdirs createallsubdirs


[Icons]

Name: "{group}\HyperBotUploader"; \
    Filename: "{app}\HyperBotUploader.exe"

Name: "{autodesktop}\HyperBotUploader"; \
    Filename: "{app}\HyperBotUploader.exe"


[Run]

Filename: "{app}\HyperBotUploader.exe"; \
    Description: "Launch HyperBotUploader"; \
    Flags: nowait postinstall skipifsilent