#include <catch2/catch_test_macros.hpp>
#include "dsp/Smoother.h"

TEST_CASE ("Smoother approaches target", "[dsp]")
{
    Smoother s;
    s.prepare (48000.0, 0.01f);
    s.setCurrentAndTarget (0.0f);
    s.setTarget (1.0f);
    float last = 0.0f;
    for (int i = 0; i < 4800; ++i)
        last = s.next();
    REQUIRE (last > 0.95f);
}
