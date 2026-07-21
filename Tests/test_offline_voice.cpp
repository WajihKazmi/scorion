#include <catch2/catch_test_macros.hpp>
#include "synth/VirtualAnalogEngine.h"
#include "synth/FmEngine.h"
#include "dsp/AdsrEnvelope.h"
#include "dsp/ZdfFilter.h"
#include "dsp/Oversampler2x.h"
#include <cmath>

TEST_CASE ("Offline VA voice produces finite audio", "[offline][dsp]")
{
    VirtualAnalogEngine eng;
    eng.prepare (48000.0, 256);
    NoteEvent n;
    n.note = 48;
    n.velocity = 1.0f;
    eng.noteOn (0, n);

    AdsrEnvelope env;
    env.prepare (48000.0);
    env.setAttack (0.01f);
    env.setDecay (0.2f);
    env.setSustain (0.6f);
    env.setRelease (0.2f);
    env.noteOn();

    MoogLadderFilter ladder;
    ladder.prepare (48000.0);
    ladder.setCutoff (2000.0f);
    ladder.setResonance (0.3f);

    ModulatedParams p;
    p.unisonCount = 3;
    p.unisonDetune = 0.2f;
    p.pulseWidth = 0.4f;

    double energy = 0.0;
    for (int i = 0; i < 48000; ++i)
    {
        float l = 0, r = 0;
        eng.render (0, &l, &r, 1, p);
        l = ladder.process (l) * env.next();
        REQUIRE (std::isfinite (l));
        energy += (double) l * (double) l;
        if (i == 24000)
            env.noteOff();
    }
    REQUIRE (energy > 10.0);
}

TEST_CASE ("FM engine offline energy", "[offline][dsp]")
{
    FmEngine eng;
    eng.prepare (48000.0, 128);
    NoteEvent n;
    n.note = 72;
    n.velocity = 0.8f;
    eng.noteOn (0, n);
    ModulatedParams p;
    p.fmRatio = 3.5f;
    p.fmIndex = 4.0f;
    double energy = 0.0;
    for (int i = 0; i < 8000; ++i)
    {
        float l = 0, r = 0;
        eng.render (0, &l, &r, 1, p);
        energy += (double) l * (double) l;
    }
    REQUIRE (energy > 1.0);
}
