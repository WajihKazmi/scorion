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
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Set-Location $Root

$BuildAbs = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $Root $BuildDir
}

function Find-ScorionVst3Bundle {
    param([string]$RootBuild)

    $candidates = @(
        (Join-Path $RootBuild "Scorion_artefacts\Release\VST3\Scorion.vst3"),
        (Join-Path $RootBuild "Scorion_artefacts\VST3\Release\Scorion.vst3"),
        (Join-Path $RootBuild "Scorion_artefacts\VST3\Scorion.vst3"),
        "${env:ProgramFiles}\Common Files\VST3\Scorion.vst3",
        "${env:CommonProgramFiles}\VST3\Scorion.vst3"
    )

    foreach ($c in $candidates) {
        if ($c -and (Test-Path -LiteralPath $c)) {
            $winBin = Join-Path $c "Contents\x86_64-win"
            if (Test-Path -LiteralPath $winBin) {
                return (Resolve-Path -LiteralPath $c).Path
            }
            # Bundle folder present (mac-style or partial) — still usable
            if (Test-Path -LiteralPath (Join-Path $c "Contents")) {
                return (Resolve-Path -LiteralPath $c).Path
            }
        }
    }

    if (Test-Path -LiteralPath $RootBuild) {
        $found = Get-ChildItem -LiteralPath $RootBuild -Recurse -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.PSIsContainer -and $_.Name -eq "Scorion.vst3" } |
            Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName "Contents") } |
            Select-Object -First 1
        if ($found) { return $found.FullName }
    }

    return $null
}

$vst3 = Find-ScorionVst3Bundle -RootBuild $BuildAbs
if (-not $vst3) {
    Write-Host "Searched under: $BuildAbs"
    if (Test-Path -LiteralPath $BuildAbs) {
        Get-ChildItem -LiteralPath $BuildAbs -Recurse -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "*Scorion*" -or $_.Name -like "*VST3*" } |
            Select-Object -First 40 FullName |
            ForEach-Object { Write-Host "  $($_.FullName)" }
    }
    throw "Scorion.vst3 not found under $BuildAbs. Build VST3 Release first."
}

$OutAbs = if ([System.IO.Path]::IsPathRooted($OutDir)) { $OutDir } else { Join-Path $Root $OutDir }
if (Test-Path -LiteralPath $OutAbs) { Remove-Item -Recurse -Force -LiteralPath $OutAbs }
New-Item -ItemType Directory -Force -Path $OutAbs | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $OutAbs "ScorionResources") | Out-Null

Write-Host "Copying VST3 from $vst3"
Copy-Item -Recurse -Force -LiteralPath $vst3 -Destination (Join-Path $OutAbs "Scorion.vst3")

# Lean resources: presets + wtbin + samples + fonts (skip duplicate wavetable .wav)
$resSrc = Join-Path $Root "Resources"
$resDst = Join-Path $OutAbs "ScorionResources"
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
$embed = Join-Path $OutAbs "Scorion.vst3\Contents\Resources\ScorionResources"
New-Item -ItemType Directory -Force -Path $embed | Out-Null
Copy-Item -Recurse -Force "$resDst\*" $embed

$size = (Get-ChildItem -Recurse $OutAbs | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host ("Packed dist\windows ({0:N1} MB)" -f $size)
Write-Host "Next: compile installer\windows\ScorionSetup.iss with Inno Setup 6"
