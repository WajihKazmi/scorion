# Release checklist (M9) — v1 complete

## Build

- [x] CMake Configure (Release, Ninja)
- [x] Standalone builds (`Scorion.app`)
- [x] VST3 builds (`Scorion.vst3`)
- [x] AU builds (`Scorion.component`)
- [x] Unit test target `ScorionTests`
- [x] Factory Resources bundled into app/plugin bundles
- [x] GitHub Actions CI workflow (`.github/workflows/ci.yml`)
- [x] Local smoke script (`scripts/smoke.sh`)

## Quality gates

- [x] Catch2: smoother, ADSR, PolyBLEP, modulation, SPSC, preset JSON
- [x] Catch2: factory wavetable load + offline VA/FM render
- [ ] pluginval on VST3 (optional host validation)
- [ ] Long soak / notarization (requires certs + time)

## Manual DAW smoke (demo day)

- [x] Standalone launches with LANDR-style UI + piano
- [ ] Ableton / Logic / Reaper: confirm VST3/AU discovery
- [x] Panic / keyboard play path implemented

## Artifacts

```
build/Scorion_artefacts/Release/Standalone/Scorion.app
build/Scorion_artefacts/Release/VST3/Scorion.vst3
build/Scorion_artefacts/Release/AU/Scorion.component
```

Bundled content lives at:

`…/Contents/Resources/ScorionResources/factory/{presets,wavetables}`

Installed copies (via `COPY_PLUGIN_AFTER_BUILD`):

- `~/Library/Audio/Plug-Ins/VST3/Scorion.vst3`
- `~/Library/Audio/Plug-Ins/Components/Scorion.component`
