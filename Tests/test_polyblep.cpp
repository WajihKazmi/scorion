#include <catch2/catch_test_macros.hpp>
#include "dsp/PolyBlepOscillator.h"
#include <cmath>

TEST_CASE ("PolyBLEP saw is finite and non-silent", "[dsp]")
{
    PolyBlepOscillator osc;
    osc.prepare (48000.0);
    osc.setWaveform (OscWaveform::Saw);
    osc.setFrequency (440.0f);
    double energy = 0.0;
    for (int i = 0; i < 4800; ++i)
    {
        const float s = osc.next();
        REQUIRE (std::isfinite (s));
        energy += (double) s * (double) s;
    }
    REQUIRE (energy > 100.0);
}
