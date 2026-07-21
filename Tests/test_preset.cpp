#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>
#include "presets/PresetManager.h"

TEST_CASE ("Preset JSON round trip", "[preset]")
{
    auto* root = new juce::DynamicObject();
    root->setProperty ("schema", 1);
    root->setProperty ("name", "Test Pad");
    root->setProperty ("category", "Pads");
    auto text = PresetManager::serializePresetJson (root);
    auto parsed = PresetManager::parsePresetJson (text);
    REQUIRE (parsed.isObject());
    REQUIRE (parsed["name"].toString() == "Test Pad");
    REQUIRE ((int) parsed["schema"] == 1);
}
