# Design System Master — Scorion

**Project:** Scorion  
**Updated:** 2026-07-22  
**Pattern:** Portal-inspired one-screen + sound library + Settings  
**Identity:** Black / white neon chrome with five named premium themes

## Visual thesis

Monochrome void with neon accents. Ring-dial knobs (track + bloom arc + needle +
tip + 0–10 scale marks). Stronger layered panel shadows. Library cards with
prominent type and PLAY badge.

## Premium themes (Settings tab)

| Theme | Feel |
|-------|------|
| Mono Neon | Void black, white neon, cyan mod |
| Obsidian Frost | Cold steel whites, ice-blue glow |
| Pure Mono | Strict black & white, no colour accent |
| Noir Phosphor | CRT phosphor green on deep black |
| Ivory Circuit | Light ivory panels, ink black type |

## Color Tokens (Mono Neon default)

| Role | Hex |
|------|-----|
| Background | `#070707` |
| Surface | `#0E0E0E` |
| Panel | `#141414` |
| Card | `#1A1A1A` |
| Border | `rgba(255,255,255,.08)` |
| Primary (white neon) | `#FFFFFF` |
| Secondary / mod (cyan) | `#00F0FF` |
| Muted | `#8A8A8A` |
| Danger | `#FF3B5C` |
| Success | `#39FF14` |

## Knobs

- Flat sober disc + thin rim
- Quiet track arc
- **Single indicator line** (muted at rest)
- On hover / hold / drag: line + tip **colour glow**
- Optional sparse 0 / 5 / 10 ticks (Settings)
- Mod params use accent colour when active; others use primary

## Settings

- Theme picker (5 named themes)
- UI scale 85–150% (FL Studio / HiDPI)
- Knob scale numbers
- Library audition on click
- FL Studio friendly mode (host programs + focus)
- Plugin MIDI channel

## Typography

Space Grotesk + Manrope — larger UI/title weights for library and labels
