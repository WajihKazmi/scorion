#!/usr/bin/env python3
"""Stamp factory presets with mood / energy / brightness / warmth / artworkSeed metadata."""
from __future__ import annotations

import json
import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PRESETS = ROOT / "Resources" / "factory" / "presets"

MOOD_BY_CAT = {
    "Dark Synths": "gothic",
    "Atmospheres": "haunting",
    "Basses": "heavy",
    "Moog Basses": "warm",
    "Pads": "ethereal",
    "Leads": "bright",
    "Pianos": "warm",
    "Bells / Scapes": "crystalline",
    "Hiphop": "gritty",
    "RnB": "smooth",
    "Soul": "soulful",
    "Pop": "bright",
    "Epic": "cinematic",
    "Vocal Synths": "vocal",
    "Samples": "organic",
}

ENERGY_BY_CAT = {
    "Dark Synths": 0.65, "Atmospheres": 0.25, "Basses": 0.7, "Moog Basses": 0.6,
    "Pads": 0.3, "Leads": 0.75, "Pianos": 0.4, "Bells / Scapes": 0.35,
    "Hiphop": 0.7, "RnB": 0.45, "Soul": 0.4, "Pop": 0.65, "Epic": 0.8,
    "Vocal Synths": 0.5, "Samples": 0.45,
}


def seed_for(name: str) -> int:
    return int(hashlib.md5(name.encode()).hexdigest()[:8], 16)


def derive(data: dict) -> dict:
    cat = data.get("category", "Pads")
    params = data.get("parameters") or {}
    cutoff = float(params.get("filterCutoff", 2000))
    reverb = float(params.get("reverbMix", 0.2))
    tags = [t.lower() for t in data.get("tags", [])]

    mood = MOOD_BY_CAT.get(cat, "neutral")
    if "dark" in tags or "horror" in tags:
        mood = "gothic"
    elif "warm" in tags or "tape" in tags:
        mood = "warm"
    elif "ambient" in tags:
        mood = "haunting"

    energy = ENERGY_BY_CAT.get(cat, 0.5)
    brightness = max(0.05, min(1.0, cutoff / 12000.0))
    warmth = max(0.05, min(1.0, 1.0 - brightness * 0.6 + reverb * 0.25))
    if "dark" in tags:
        brightness *= 0.55
        warmth = min(1.0, warmth + 0.15)

    data["mood"] = mood
    data["energy"] = round(energy, 3)
    data["brightness"] = round(brightness, 3)
    data["warmth"] = round(warmth, 3)
    data["artworkSeed"] = seed_for(data.get("name", "x"))
    if "author" not in data or not data["author"]:
        data["author"] = "Scorion Factory"
    return data


def main():
    n = 0
    for path in sorted(PRESETS.glob("*.json")):
        data = json.loads(path.read_text())
        if not isinstance(data, dict):
            continue
        data = derive(data)
        path.write_text(json.dumps(data, indent=2) + "\n")
        n += 1
    print(f"Enriched {n} presets")


if __name__ == "__main__":
    main()
