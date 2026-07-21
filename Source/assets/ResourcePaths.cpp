#include "assets/ResourcePaths.h"

namespace ResourcePaths
{
namespace
{
juce::File candidateFromExe()
{
    const auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    // App: .../Scorion.app/Contents/MacOS/Scorion -> Resources/ScorionResources
    auto appRes = exe.getParentDirectory().getParentDirectory().getChildFile ("Resources")
                      .getChildFile ("ScorionResources");
    if (appRes.getChildFile ("factory").isDirectory())
        return appRes;

    // VST3/AU mac: .../Contents/MacOS/Scorion -> Resources/ScorionResources
    auto plugRes = exe.getParentDirectory().getParentDirectory().getChildFile ("Resources")
                       .getChildFile ("ScorionResources");
    if (plugRes.getChildFile ("factory").isDirectory())
        return plugRes;

    // Windows VST3: .../Contents/x86_64-win/Scorion.vst3 -> ../Resources/ScorionResources
    auto winRes = exe.getParentDirectory().getParentDirectory().getChildFile ("Resources")
                      .getChildFile ("ScorionResources");
    if (winRes.getChildFile ("factory").isDirectory())
        return winRes;

    // Installer layout: Common Files/VST3/Scorion.vst3 + sibling ScorionResources
    auto sibling = exe.getParentDirectory().getParentDirectory().getParentDirectory()
                       .getChildFile ("ScorionResources");
    if (sibling.getChildFile ("factory").isDirectory())
        return sibling;

    // User install fallback
    auto user = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("Scorion")
                    .getChildFile ("Resources");
    if (user.getChildFile ("factory").isDirectory())
        return user;

    return {};
}

juce::File candidateFromSourceTree()
{
    // Source/assets/ResourcePaths.cpp -> repo root Resources
    auto repo = juce::File (__FILE__).getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile ("Resources");
    if (repo.getChildFile ("factory").isDirectory())
        return repo;
    return {};
}
} // namespace

juce::File factoryRoot()
{
    if (auto c = candidateFromExe(); c.isDirectory())
        return c;
    if (auto c = candidateFromSourceTree(); c.isDirectory())
        return c;
    return juce::File();
}

juce::File factoryPresets()
{
    return factoryRoot().getChildFile ("factory").getChildFile ("presets");
}

juce::File factoryWavetables()
{
    return factoryRoot().getChildFile ("factory").getChildFile ("wavetables");
}

juce::File factorySamples()
{
    return factoryRoot().getChildFile ("factory").getChildFile ("samples");
}

juce::File userPresets()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Scorion")
                   .getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}
} // namespace ResourcePaths
