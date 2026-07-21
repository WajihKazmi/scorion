# Packs a lean Windows VST3 tree for the Inno Setup installer.
# Run on Windows after a Release VST3 build, or from CI.
#
# Usage (from repo root, PowerShell):
#   .\installer\windows\pack_dist.ps1 -BuildDir build

param(
    [string]$BuildDir = "build",
    [string]$OutDir = "dist\windows"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Set-Location $Root

$vst3Candidates = @(
    Join-Path $BuildDir "Scorion_artefacts\Release\VST3\Scorion.vst3",
    Join-Path $BuildDir "Scorion_artefacts\VST3\Scorion.vst3"
)

$vst3 = $null
foreach ($c in $vst3Candidates) {
    if (Test-Path $c) { $vst3 = $c; break }
}
if (-not $vst3) {
    throw "Scorion.vst3 not found under $BuildDir. Build VST3 Release first."
}

if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $OutDir "ScorionResources") | Out-Null

Write-Host "Copying VST3 from $vst3"
Copy-Item -Recurse -Force $vst3 (Join-Path $OutDir "Scorion.vst3")

# Lean resources: presets + wtbin + samples + fonts (skip duplicate wavetable .wav)
$resSrc = Join-Path $Root "Resources"
$resDst = Join-Path $OutDir "ScorionResources"
Copy-Item -Force (Join-Path $resSrc "LICENSES.md") $resDst -ErrorAction SilentlyContinue
Copy-Item -Recurse -Force (Join-Path $resSrc "fonts") (Join-Path $resDst "fonts")
New-Item -ItemType Directory -Force -Path (Join-Path $resDst "factory") | Out-Null
Copy-Item -Force (Join-Path $resSrc "factory\*.json") (Join-Path $resDst "factory") -ErrorAction SilentlyContinue
Copy-Item -Recurse -Force (Join-Path $resSrc "factory\presets") (Join-Path $resDst "factory\presets")
Copy-Item -Recurse -Force (Join-Path $resSrc "factory\samples") (Join-Path $resDst "factory\samples")
New-Item -ItemType Directory -Force -Path (Join-Path $resDst "factory\wavetables") | Out-Null
Get-ChildItem (Join-Path $resSrc "factory\wavetables\*.wtbin") | ForEach-Object {
    Copy-Item -Force $_.FullName (Join-Path $resDst "factory\wavetables")
}

# Also embed resources inside the VST3 bundle for self-contained load
$embed = Join-Path $OutDir "Scorion.vst3\Contents\Resources\ScorionResources"
New-Item -ItemType Directory -Force -Path $embed | Out-Null
Copy-Item -Recurse -Force "$resDst\*" $embed

$size = (Get-ChildItem -Recurse $OutDir | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ("Packed dist\windows ({0:N1} MB)" -f $size)
Write-Host "Next: compile installer\windows\ScorionSetup.iss with Inno Setup 6"
