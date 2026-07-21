; Scorion Windows installer — one-run VST3 install for FL Studio
; Build with Inno Setup 6 after compiling the VST3 on Windows.
; Expected layout (from CI / packaging script):
;   dist/windows/Scorion.vst3/...
;   dist/windows/ScorionResources/...

#define MyAppName "Scorion"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Wajih Kazmi"
#define MyAppURL "https://github.com/WajihKazmi/scorion"
#define DistDir "..\..\dist\windows"

[Setup]
AppId={{8F3C2A91-7B4E-4D1A-9C6F-A1B2C3D4E5F6}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={commoncf64}\VST3
DefaultGroupName=Scorion
DisableProgramGroupPage=yes
DisableDirPage=yes
OutputDir=..\..\dist
OutputBaseFilename=Scorion-Setup-Windows
Compression=lzma2/ultra64
SolidCompression=yes
LZMAUseSeparateProcess=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\Scorion.vst3
SetupIconFile=
InfoBeforeFile=
LicenseFile=

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 bundle (FL Studio / all VST3 hosts scan Common Files\VST3)
Source: "{#DistDir}\Scorion.vst3\*"; DestDir: "{commoncf64}\VST3\Scorion.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
; Factory bank next to plugin (also copied to user AppData as fallback)
Source: "{#DistDir}\ScorionResources\*"; DestDir: "{commoncf64}\VST3\Scorion.vst3\Contents\Resources\ScorionResources"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\ScorionResources\*"; DestDir: "{userappdata}\Scorion\Resources"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall Scorion"; Filename: "{uninstallexe}"

[Run]
Filename: "{cmd}"; Parameters: "/C echo Scorion installed. Open FL Studio → Plugins → refresh / find Scorion."; Flags: runhidden nowait

[Code]
function InitializeWizard(): Boolean;
begin
  Result := True;
  WizardForm.WelcomeLabel2.Caption :=
    'This installs Scorion as a VST3 instrument for FL Studio and other DAWs.' + #13#10 + #13#10 +
    'Destination:' + #13#10 +
    '  C:\Program Files\Common Files\VST3\Scorion.vst3' + #13#10 + #13#10 +
    'After install: open FL Studio → Plugin Manager → find Scorion (or refresh).';
end;
