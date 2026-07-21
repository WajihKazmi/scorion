#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo "==> Configure"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja

echo "==> Build"
cmake --build build --parallel

echo "==> Unit tests"
./build/ScorionTests

echo "==> Artifact checks"
test -d build/Scorion_artefacts/Release/Standalone/Scorion.app
test -d build/Scorion_artefacts/Release/VST3/Scorion.vst3
test -d build/Scorion_artefacts/Release/AU/Scorion.component

RES="build/Scorion_artefacts/Release/Standalone/Scorion.app/Contents/Resources/ScorionResources"
if [[ -d "$RES/factory/presets" ]]; then
  echo "Bundled presets: $(ls "$RES/factory/presets" | wc -l | tr -d ' ')"
  echo "Bundled wavetables: $(ls "$RES/factory/wavetables"/*.wtbin 2>/dev/null | wc -l | tr -d ' ')"
else
  echo "WARN: Resources not yet bundled (rebuild Standalone target)"
fi

echo "==> Smoke OK — launch with:"
echo "open build/Scorion_artefacts/Release/Standalone/Scorion.app"
