#pragma once

#include <juce_core/juce_core.h>

/** Resolve Scorion factory Resources across source tree, app bundle, and install layouts. */
namespace ResourcePaths
{
    /** Directory containing factory/presets and factory/wavetables. */
    juce::File factoryRoot();

    juce::File factoryPresets();
    juce::File factoryWavetables();
    juce::File factorySamples();
    juce::File userPresets();
}
