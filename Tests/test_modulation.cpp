#include <catch2/catch_test_macros.hpp>
#include "modulation/ModulationMatrix.h"

TEST_CASE ("Modulation matrix connection affects cutoff", "[mod]")
{
    ModulationMatrix m;
    m.prepare (48000.0);
    m.clearConnections();
    REQUIRE (m.addConnection (ModSource::Macro1, ModDest::Cutoff, 1.0f, true));
    m.setMacro (0, 1.0f);
    ModulationMatrix::VoiceMod vm;
    m.tickVoice (0, vm);
    REQUIRE (std::abs (vm.cutoff) > 0.1f);
}
