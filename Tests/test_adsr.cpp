#include <catch2/catch_test_macros.hpp>
#include "dsp/AdsrEnvelope.h"

TEST_CASE ("ADSR note cycle", "[dsp]")
{
    AdsrEnvelope e;
    e.prepare (48000.0);
    e.setAttack (0.01f);
    e.setDecay (0.05f);
    e.setSustain (0.5f);
    e.setRelease (0.05f);
    e.noteOn();
    float peak = 0.0f;
    for (int i = 0; i < 4800; ++i)
        peak = std::max (peak, e.next());
    REQUIRE (peak > 0.9f);
    e.noteOff();
    float last = 1.0f;
    for (int i = 0; i < 20000; ++i)
        last = e.next();
    REQUIRE (last < 0.01f);
    REQUIRE_FALSE (e.isActive());
}
