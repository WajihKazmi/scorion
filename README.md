# Scorion

Flagship virtual instrument by **Wajih Kazmi** — JUCE 8 / C++20. **VST3 · AU · Standalone**.

One repo. Full source, factory bank, design system, Windows FL Studio installer scripts, and CI.

## Quick start (macOS / Linux)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --parallel
./build/ScorionTests
open build/Scorion_artefacts/Release/Standalone/Scorion.app   # macOS
```

## Windows + FL Studio (one-run install)

1. Download **Scorion-Setup-Windows.exe** from [Releases](https://github.com/WajihKazmi/scorion/releases).
2. Run the installer once.
3. Open FL Studio → Plugin Manager → find **Scorion** (VST3 instrument).

Installer details: [`installer/windows/README.md`](installer/windows/README.md).

CI builds the Setup.exe on every push to `main` (artifact **scorion-windows-setup**).

## What’s included

| Area | Detail |
| --- | --- |
| Engines | Virtual Analog, Wavetable, FM, Granular, Sampler |
| Library | Factory presets, favorites, audition, smart search |
| UI | Play + Settings, 5 premium themes, sober glow knobs |
| Host | VST3 programs for FL Studio, HiDPI UI scale |
| Tests / CI | Catch2 + macOS + Windows installer workflows |

## Docs

- [`docs/ENGINEERING_SPEC.md`](docs/ENGINEERING_SPEC.md)
- [`docs/PROGRESS.md`](docs/PROGRESS.md)
- [`design-system/scorion/MASTER.md`](design-system/scorion/MASTER.md)
- [`Resources/LICENSES.md`](Resources/LICENSES.md)

## License notes

Factory wavetables/samples are original. See `Resources/LICENSES.md`.
