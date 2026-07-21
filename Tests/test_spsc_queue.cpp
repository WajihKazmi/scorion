#include <catch2/catch_test_macros.hpp>
#include "audio/SpscQueue.h"

TEST_CASE ("SPSC queue push pop", "[audio]")
{
    SpscQueue<int, 8> q;
    REQUIRE (q.push (1));
    REQUIRE (q.push (2));
    int v = 0;
    REQUIRE (q.pop (v));
    REQUIRE (v == 1);
    REQUIRE (q.pop (v));
    REQUIRE (v == 2);
    REQUIRE_FALSE (q.pop (v));
}
