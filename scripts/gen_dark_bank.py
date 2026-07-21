#!/usr/bin/env python3
"""Generate dark synth / bass / atmosphere factory presets for Scorion."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Resources" / "factory" / "presets"
MANIFEST = ROOT / "Resources" / "factory" / "premium_bank_manifest.json"

ENGINES = {
    "Virtual Analog": 0.0,
    "Wavetable": 1.0,
    "FM": 2.0,
    "Granular": 3.0,
    "Sampler": 4.0,
}

BASE = {
    "masterGain": 0.72,
    "filterCutoff": 1200,
    "filterResonance": 0.35,
    "ampAttack": 0.01,
    "ampDecay": 0.3,
    "ampSustain": 0.7,
    "ampRelease": 0.45,
    "filterEnvAmount": 0.35,
    "pulseWidth": 0.45,
    "unisonDetune": 0.18,
    "unisonCount": 1,
    "wtPosition": 0.35,
    "fmRatio": 2.0,
    "fmIndex": 1.6,
    "grainPosition": 0.3,
    "grainSize": 0.12,
    "grainDensity": 0.45,
    "reverbMix": 0.2,
    "delayMix": 0.15,
    "macro1": 0.5,
    "macro2": 0.45,
    "macro3": 0.5,
    "macro4": 0.55,
    "lfo1Rate": 0.4,
    "lfo1ToCutoff": 0.2,
}


def preset(name: str, category: str, engine: str, tags: list[str], genres: list[str], **params):
    eng = ENGINES[engine]
    p = dict(BASE)
    p.update(params)
    p["engine"] = eng
    return {
        "schema": 1,
        "name": name,
        "category": category,
        "author": "Scorion Factory",
        "engine": engine,
        "engineIndex": int(eng),
        "tags": tags,
        "genres": genres,
        "parameters": p,
    }


# 12 Dark Synths · 10 Basses · 10 Atmospheres
BANK = [
    # --- Dark Synths ---
    preset("Blood Neon", "Dark Synths", "Wavetable",
           ["dark", "neon", "lead"], ["electronic", "trap"],
           filterCutoff=1800, filterResonance=0.55, wtPosition=0.72, unisonCount=3,
           unisonDetune=0.28, lfo1Rate=4.2, lfo1ToCutoff=0.45, delayMix=0.32, reverbMix=0.22),
    preset("Purple Static", "Dark Synths", "Virtual Analog",
           ["dark", "pwm", "noise"], ["electronic"],
           filterCutoff=900, pulseWidth=0.12, filterResonance=0.62, ampAttack=0.002,
           lfo1Rate=6.5, lfo1ToCutoff=0.55, delayMix=0.25),
    preset("Crimson Grid", "Dark Synths", "FM",
           ["dark", "fm", "metallic"], ["electronic", "epic"],
           filterCutoff=2400, fmRatio=3.5, fmIndex=4.2, ampDecay=0.45, ampSustain=0.55,
           reverbMix=0.28, delayMix=0.2),
    preset("Night Terror", "Dark Synths", "Virtual Analog",
           ["dark", "horror", "drone"], ["cinematic", "trap"],
           filterCutoff=420, filterResonance=0.7, ampAttack=0.8, ampRelease=1.4,
           unisonCount=4, unisonDetune=0.35, lfo1Rate=0.15, lfo1ToCutoff=0.4, reverbMix=0.55),
    preset("Hex Pulse", "Dark Synths", "Wavetable",
           ["dark", "pulse", "arp"], ["electronic"],
           filterCutoff=1600, wtPosition=0.55, ampAttack=0.001, ampDecay=0.18, ampSustain=0.35,
           ampRelease=0.2, filterEnvAmount=0.55, delayMix=0.4, lfo1Rate=8.0, lfo1ToCutoff=0.25),
    preset("Obsidian Saw", "Dark Synths", "Virtual Analog",
           ["dark", "saw", "heavy"], ["metal", "electronic"],
           filterCutoff=1100, filterResonance=0.48, unisonCount=5, unisonDetune=0.32,
           ampAttack=0.005, reverbMix=0.18, delayMix=0.1),
    preset("Witch House Keys", "Dark Synths", "Wavetable",
           ["dark", "keys", "witch"], ["electronic", "hiphop"],
           filterCutoff=1400, wtPosition=0.4, ampAttack=0.02, ampRelease=0.9,
           reverbMix=0.45, delayMix=0.35, lfo1Rate=0.35, lfo1ToCutoff=0.3),
    preset("Ritual Stab", "Dark Synths", "FM",
           ["dark", "stab", "hit"], ["trap", "hiphop"],
           filterCutoff=2800, fmRatio=1.5, fmIndex=3.8, ampAttack=0.001, ampDecay=0.22,
           ampSustain=0.15, ampRelease=0.25, filterEnvAmount=0.7, reverbMix=0.25),
    preset("Glass Abyss", "Dark Synths", "Granular",
           ["dark", "glass", "texture"], ["ambient", "electronic"],
           filterCutoff=2000, grainSize=0.08, grainDensity=0.7, grainPosition=0.55,
           ampAttack=0.4, ampRelease=1.2, reverbMix=0.62, delayMix=0.28),
    preset("Neon Coffin", "Dark Synths", "Wavetable",
           ["dark", "neon", "pad"], ["electronic", "rnb"],
           filterCutoff=800, wtPosition=0.85, unisonCount=3, ampAttack=0.5,
           ampRelease=1.1, reverbMix=0.5, delayMix=0.4, lfo1Rate=0.25, lfo1ToCutoff=0.35),
    preset("Black Mass Lead", "Dark Synths", "Virtual Analog",
           ["dark", "lead", "scream"], ["metal", "electronic"],
           filterCutoff=3200, filterResonance=0.58, pulseWidth=0.3, ampAttack=0.01,
           ampSustain=0.85, filterEnvAmount=0.5, delayMix=0.3, reverbMix=0.2),
    preset("Voltage Cult", "Dark Synths", "FM",
           ["dark", "voltage", "mod"], ["electronic"],
           filterCutoff=1700, fmRatio=2.7, fmIndex=5.0, lfo1Rate=5.5, lfo1ToCutoff=0.5,
           delayMix=0.22, reverbMix=0.18),

    # --- Basses ---
    preset("Void Sub", "Basses", "Virtual Analog",
           ["bass", "sub", "dark"], ["trap", "electronic"],
           filterCutoff=280, filterResonance=0.25, ampAttack=0.005, ampSustain=1.0,
           ampRelease=0.35, unisonCount=1, reverbMix=0.05, delayMix=0.0, lfo1ToCutoff=0.05),
    preset("Blood Reese", "Basses", "Virtual Analog",
           ["bass", "reese", "dark"], ["trap", "drill"],
           filterCutoff=480, filterResonance=0.55, unisonCount=6, unisonDetune=0.48,
           ampAttack=0.02, lfo1Rate=0.55, lfo1ToCutoff=0.35, reverbMix=0.1),
    preset("Purple Moog Growl", "Basses", "Virtual Analog",
           ["bass", "moog", "growl"], ["funk", "electronic"],
           filterCutoff=650, filterResonance=0.72, filterEnvAmount=0.65, ampAttack=0.01,
           ampDecay=0.35, ampSustain=0.55, pulseWidth=0.4, lfo1Rate=2.2, lfo1ToCutoff=0.4),
    preset("Dungeon 808", "Basses", "Virtual Analog",
           ["bass", "808", "dark"], ["trap", "hiphop"],
           filterCutoff=350, filterResonance=0.4, ampAttack=0.001, ampDecay=0.55,
           ampSustain=0.0, ampRelease=0.55, filterEnvAmount=0.45, reverbMix=0.08),
    preset("FM Abyss Bass", "Basses", "FM",
           ["bass", "fm", "dark"], ["electronic", "dnb"],
           filterCutoff=700, fmRatio=1.0, fmIndex=2.8, ampAttack=0.01, ampSustain=0.8,
           reverbMix=0.12, delayMix=0.08, lfo1Rate=0.8, lfo1ToCutoff=0.22),
    preset("Granular Dirt Bass", "Basses", "Granular",
           ["bass", "granular", "grit"], ["experimental", "trap"],
           filterCutoff=550, grainSize=0.2, grainDensity=0.55, grainPosition=0.2,
           ampAttack=0.03, reverbMix=0.2, delayMix=0.15),
    preset("Nightclub Pressure", "Basses", "Wavetable",
           ["bass", "club", "dark"], ["electronic", "techno"],
           filterCutoff=420, wtPosition=0.25, unisonCount=3, unisonDetune=0.22,
           ampAttack=0.005, filterEnvAmount=0.4, lfo1Rate=0.3, lfo1ToCutoff=0.18),
    preset("Shadow Slide", "Basses", "Virtual Analog",
           ["bass", "portamento", "dark"], ["rnb", "trap"],
           filterCutoff=580, filterResonance=0.45, ampAttack=0.08, ampRelease=0.7,
           unisonCount=2, unisonDetune=0.15, reverbMix=0.15, delayMix=0.2),
    preset("Inferno Sub", "Basses", "Virtual Analog",
           ["bass", "sub", "heavy"], ["metal", "trap"],
           filterCutoff=220, filterResonance=0.3, ampAttack=0.002, ampSustain=0.95,
           ampRelease=0.5, filterEnvAmount=0.25, reverbMix=0.08),
    preset("Toxic Wobble", "Basses", "Virtual Analog",
           ["bass", "wobble", "dubstep"], ["electronic", "dubstep"],
           filterCutoff=900, filterResonance=0.65, unisonCount=4, unisonDetune=0.3,
           lfo1Rate=3.5, lfo1ToCutoff=0.7, ampAttack=0.01, reverbMix=0.15),

    # --- Atmospheres ---
    preset("Void Cathedral", "Atmospheres", "Granular",
           ["atmosphere", "dark", "space"], ["ambient", "cinematic"],
           filterCutoff=900, grainSize=0.28, grainDensity=0.35, grainPosition=0.6,
           ampAttack=2.0, ampSustain=0.85, ampRelease=2.5, reverbMix=0.75, delayMix=0.45,
           lfo1Rate=0.08, lfo1ToCutoff=0.25),
    preset("Red Fog", "Atmospheres", "Wavetable",
           ["atmosphere", "pad", "dark"], ["ambient", "trap"],
           filterCutoff=700, wtPosition=0.9, ampAttack=1.5, ampRelease=2.0,
           unisonCount=4, unisonDetune=0.4, reverbMix=0.68, delayMix=0.38, lfo1Rate=0.12),
    preset("Abandoned Station", "Atmospheres", "FM",
           ["atmosphere", "industrial", "dark"], ["cinematic", "electronic"],
           filterCutoff=1100, fmRatio=4.0, fmIndex=1.2, ampAttack=1.8, ampRelease=2.2,
           reverbMix=0.7, delayMix=0.5, lfo1Rate=0.2, lfo1ToCutoff=0.3),
    preset("Purple Dusk", "Atmospheres", "Wavetable",
           ["atmosphere", "dusk", "pad"], ["ambient", "rnb"],
           filterCutoff=1300, wtPosition=0.65, ampAttack=1.2, ampRelease=1.8,
           reverbMix=0.6, delayMix=0.42, lfo1Rate=0.18, lfo1ToCutoff=0.28),
    preset("Black Horizon", "Atmospheres", "Virtual Analog",
           ["atmosphere", "drone", "dark"], ["cinematic", "ambient"],
           filterCutoff=500, filterResonance=0.2, ampAttack=2.5, ampSustain=1.0,
           ampRelease=3.0, unisonCount=3, unisonDetune=0.25, reverbMix=0.8, delayMix=0.35),
    preset("Crypt Wind", "Atmospheres", "Granular",
           ["atmosphere", "wind", "horror"], ["cinematic", "horror"],
           filterCutoff=1600, grainSize=0.15, grainDensity=0.8, grainPosition=0.75,
           ampAttack=0.9, ampRelease=1.6, reverbMix=0.65, delayMix=0.3, lfo1Rate=0.4),
    preset("Neon Rainscape", "Atmospheres", "Granular",
           ["atmosphere", "rain", "neon"], ["ambient", "electronic"],
           filterCutoff=2200, grainSize=0.06, grainDensity=0.9, grainPosition=0.4,
           ampAttack=0.6, ampRelease=1.4, reverbMix=0.55, delayMix=0.55, lfo1Rate=0.5),
    preset("Deep Orbit Dark", "Atmospheres", "Wavetable",
           ["atmosphere", "space", "dark"], ["ambient", "epic"],
           filterCutoff=850, wtPosition=0.5, ampAttack=2.2, ampRelease=2.8,
           unisonCount=5, unisonDetune=0.35, reverbMix=0.72, delayMix=0.4),
    preset("Sacrifice Pad", "Atmospheres", "Virtual Analog",
           ["atmosphere", "ritual", "pad"], ["cinematic", "dark"],
           filterCutoff=600, pulseWidth=0.55, ampAttack=1.6, ampRelease=2.4,
           filterResonance=0.35, reverbMix=0.7, delayMix=0.28, lfo1Rate=0.1, lfo1ToCutoff=0.2),
    preset("Afterlife Hum", "Atmospheres", "FM",
           ["atmosphere", "hum", "ghost"], ["ambient", "horror"],
           filterCutoff=1000, fmRatio=1.25, fmIndex=0.9, ampAttack=1.4, ampRelease=2.0,
           reverbMix=0.78, delayMix=0.48, lfo1Rate=0.22, lfo1ToCutoff=0.35),
]


def slug(name: str) -> str:
    return "".join(c if c.isalnum() else "_" for c in name.lower()).strip("_")


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    written = []
    for item in BANK:
        path = OUT / f"{slug(item['name'])}.json"
        path.write_text(json.dumps(item, indent=2) + "\n")
        written.append(path.name)

    # Recount categories from all presets on disk
    cats: dict[str, int] = {}
    for f in sorted(OUT.glob("*.json")):
        data = json.loads(f.read_text())
        cat = data.get("category", "Other")
        cats[cat] = cats.get(cat, 0) + 1

    manifest = {
        "schema": 1,
        "count": sum(cats.values()),
        "bank": "Scorion Void Neon Factory",
        "categories": dict(sorted(cats.items(), key=lambda kv: (-kv[1], kv[0]))),
        "addedThisPass": written,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Wrote {len(written)} presets; factory total {manifest['count']}")
    for k, v in manifest["categories"].items():
        print(f"  {k}: {v}")


if __name__ == "__main__":
    main()
