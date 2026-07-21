#!/usr/bin/env python3
"""Generate original Scorion factory wavetables (.wtbin + .wav). MIT-owned assets."""
import json, math, struct, wave
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[1] / "Resources" / "factory" / "wavetables"
FRAME, FRAMES, SR = 2048, 256, 44100


def bandlimited_saw(phase, n_harm):
    s = np.zeros_like(phase)
    for h in range(1, n_harm + 1):
        s += np.sin(2 * np.pi * h * phase) / h
    return s


def bandlimited_square(phase, n_harm):
    s = np.zeros_like(phase)
    for h in range(1, n_harm + 1, 2):
        s += np.sin(2 * np.pi * h * phase) / h
    return s


def normalize(x, peak=0.89):
    return (x / (np.max(np.abs(x)) + 1e-12)) * peak


def make_table(kind):
    phase = np.linspace(0, 1, FRAME, endpoint=False)
    table = np.zeros((FRAMES, FRAME), dtype=np.float32)
    for f in range(FRAMES):
        t = f / (FRAMES - 1)
        if kind == "harmonic_evolve":
            table[f] = normalize(bandlimited_saw(phase, int(2 + t * 64)))
        elif kind == "square_morph":
            n = int(3 + t * 48)
            pw = 0.15 + 0.7 * t
            table[f] = normalize(bandlimited_saw(phase, n) - bandlimited_saw((phase + pw) % 1.0, n))
        elif kind == "formant_vox":
            f1, f2, f3 = 300 + 500 * t, 800 + 1400 * t, 2200 + 800 * (1 - t)
            s = np.zeros(FRAME)
            for h in range(1, 40):
                freq = h * 220.0
                amp = math.exp(-0.5 * ((freq - f1) / 180) ** 2)
                amp += 0.7 * math.exp(-0.5 * ((freq - f2) / 250) ** 2)
                amp += 0.4 * math.exp(-0.5 * ((freq - f3) / 300) ** 2)
                s += amp * np.sin(2 * np.pi * h * phase) / (h ** 0.6)
            table[f] = normalize(s)
        elif kind == "metallic":
            s = np.zeros(FRAME)
            for i, r in enumerate([1, 1.41, 2.12, 2.83, 3.61, 4.73]):
                for h in range(1, max(1, int(24 * (1 - 0.5 * t) / (i + 1))) + 1):
                    s += np.sin(2 * np.pi * (r * h) * phase) / (h * (1 + i * 0.35))
            s += 0.15 * t * np.sin(2 * np.pi * phase * (8 + 40 * t))
            table[f] = normalize(s)
        elif kind == "glass_pad":
            s = np.zeros(FRAME)
            for h in range(1, int(8 + 40 * t)):
                s += np.sin(2 * np.pi * h * phase + 0.3 * math.sin(h)) / (h ** 1.2)
            table[f] = normalize(s + 0.2 * np.sin(2 * np.pi * phase * 3.01))
        elif kind == "analog_warm":
            table[f] = normalize(np.tanh(bandlimited_saw(phase, int(4 + (1 - t) * 28)) * (1.2 + t)))
        elif kind == "digital_harsh":
            s = bandlimited_square(phase, int(8 + t * 80))
            table[f] = normalize(np.sign(s) * (np.abs(s) ** (0.6 + 0.4 * (1 - t))))
        elif kind == "noise_bitcrush":
            rng = np.random.default_rng(f + 7)
            base = bandlimited_saw(phase, int(6 + 20 * t)) + rng.normal(0, 0.15 + 0.35 * t, FRAME)
            steps = 2 ** int(4 + 8 * (1 - t))
            table[f] = normalize(np.round(base * steps) / steps)
    return table


def main():
    ROOT.mkdir(parents=True, exist_ok=True)
    kinds = [
        "harmonic_evolve", "square_morph", "formant_vox", "metallic",
        "glass_pad", "analog_warm", "digital_harsh", "noise_bitcrush",
    ]
    manifest = {"schema": 1, "frameSize": FRAME, "numFrames": FRAMES, "tables": []}
    for kind in kinds:
        pcm = make_table(kind).reshape(-1)
        with open(ROOT / f"{kind}.wtbin", "wb") as f:
            f.write(struct.pack("<ii", FRAME, FRAMES))
            f.write(pcm.astype("<f4").tobytes())
        with wave.open(str(ROOT / f"{kind}.wav"), "w") as w:
            w.setnchannels(1)
            w.setsampwidth(2)
            w.setframerate(SR)
            w.writeframes((np.clip(pcm, -1, 1) * 32767).astype(np.int16).tobytes())
        manifest["tables"].append({"id": kind, "file": f"{kind}.wtbin", "wav": f"{kind}.wav"})
        print("wrote", kind)
    (ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
