#include <catch2/catch_test_macros.hpp>
#include "synth/WavetableEngine.h"
#include <fstream>
#include <vector>
#include <cstdint>

TEST_CASE ("Wavetable engine loads factory wtbin", "[wt][assets]")
{
#ifdef SCORION_REPO_ROOT
    const std::string path = std::string (SCORION_REPO_ROOT) + "/Resources/factory/wavetables/glass_pad.wtbin";
#else
    const std::string path = "Resources/factory/wavetables/glass_pad.wtbin";
#endif
    std::ifstream in (path, std::ios::binary);
    REQUIRE (in.good());
    in.seekg (0, std::ios::end);
    const auto size = (size_t) in.tellg();
    in.seekg (0, std::ios::beg);
    std::vector<uint8_t> bytes (size);
    in.read (reinterpret_cast<char*> (bytes.data()), (std::streamsize) size);
    REQUIRE (in.good());

    WavetableEngine eng;
    REQUIRE (eng.loadWtBin (bytes.data(), bytes.size()));
    eng.prepare (48000.0, 128);

    NoteEvent note;
    note.note = 60;
    note.velocity = 0.9f;
    eng.noteOn (0, note);

    ModulatedParams p;
    p.wtPosition = 0.4f;
    p.unisonCount = 1;
    float l = 0, r = 0;
    double energy = 0.0;
    for (int i = 0; i < 2048; ++i)
    {
        eng.render (0, &l, &r, 1, p);
        energy += (double) l * (double) l;
    }
    REQUIRE (energy > 1.0);
}
