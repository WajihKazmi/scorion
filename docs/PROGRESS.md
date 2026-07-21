# Progress

Phased implementation against [`ENGINEERING_SPEC.md`](ENGINEERING_SPEC.md).

| Milestone | Status | Notes |
| --- | --- | --- |
| M1 Core framework | Complete | CMake, JUCE 8, AudioEngine, APVTS, SPSC, Catch2 |
| M2 Oscillators & voices | Complete | PolyBLEP VA, 16 voices, steal, ADSR |
| M3 Filters & modulation | Complete | TPT ZDF + Moog ladder, LFO/matrix/macros, MPE, MIDI learn |
| M4 Effects | Complete | EQ, chorus, 2x OS sat, delay, FDN reverb, limiter |
| M5 Sampler & granular | Complete | Builtin sample, grain pool, asset loader |
| M6 Wavetable & FM | Complete | Mipmapped WT + factory `.wtbin`, FM engine |
| M7 Preset browser | Complete | JSON I/O, 31 factory presets, user save |
| M8 UI polish | Complete | LANDR-class navy/cyan UI, spectrum, piano keyboard |
| M9 Optimization & testing | Complete | Unit + offline tests, CI, bundled Resources, smoke script |

## v1 product delivered

- Formats: Standalone, VST3, AU (macOS)
- Factory wavetables + cinematic/soundscape presets bundled into each artifact
- Demo UI with playable on-screen keyboard
- AI deferred (spec Appendix A)

## Remaining post-v1 (not blockers)

- pluginval in CI, Apple notarization, Windows packaging
- Optional: table browser UI, convolution IRs, multi-layer sampler
