#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "assets/ResourcePaths.h"

ScorionAudioProcessor::ScorionAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout()),
      presetManager()
{
    presetManager.initialise (ResourcePaths::factoryPresets(), ResourcePaths::userPresets());
    engine.assets().start();
}

ScorionAudioProcessor::~ScorionAudioProcessor()
{
    engine.assets().shutdown();
}

juce::AudioProcessorValueTreeState::ParameterLayout ScorionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "engine", 1 }, "Engine",
        juce::StringArray { "Virtual Analog", "Wavetable", "FM", "Granular", "Sampler" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "masterGain", 1 }, "Master",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f, 0.5f }, 0.75f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "filterCutoff", 1 }, "Filter Cutoff",
        juce::NormalisableRange<float> { 40.0f, 18000.0f, 0.1f, 0.3f }, 6000.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "filterResonance", 1 }, "Filter Resonance",
        juce::NormalisableRange<float> { 0.0f, 0.95f, 0.001f }, 0.2f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "ampAttack", 1 }, "Attack",
        juce::NormalisableRange<float> { 0.001f, 4.0f, 0.001f, 0.4f }, 0.01f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "ampDecay", 1 }, "Decay",
        juce::NormalisableRange<float> { 0.001f, 4.0f, 0.001f, 0.4f }, 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "ampSustain", 1 }, "Sustain",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.7f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "ampRelease", 1 }, "Release",
        juce::NormalisableRange<float> { 0.001f, 8.0f, 0.001f, 0.4f }, 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "filterEnvAmount", 1 }, "Filter Env",
        juce::NormalisableRange<float> { -1.0f, 1.0f, 0.001f }, 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "pulseWidth", 1 }, "Pulse Width",
        juce::NormalisableRange<float> { 0.05f, 0.95f, 0.001f }, 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "unisonDetune", 1 }, "Unison Detune",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.15f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "unisonCount", 1 }, "Unison", 1, 8, 1));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "wtPosition", 1 }, "WT Position",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "fmRatio", 1 }, "FM Ratio",
        juce::NormalisableRange<float> { 0.25f, 8.0f, 0.01f }, 2.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "fmIndex", 1 }, "FM Index",
        juce::NormalisableRange<float> { 0.0f, 10.0f, 0.01f }, 1.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainPosition", 1 }, "Grain Position",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.2f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainSize", 1 }, "Grain Size",
        juce::NormalisableRange<float> { 0.01f, 0.4f, 0.001f }, 0.08f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "grainDensity", 1 }, "Grain Density",
        juce::NormalisableRange<float> { 0.05f, 1.0f, 0.001f }, 0.45f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "reverbMix", 1 }, "Reverb",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.18f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "delayMix", 1 }, "Delay",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.12f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "macro1", 1 }, "Macro 1",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.5f));

    for (int m = 2; m <= 4; ++m)
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "macro" + juce::String (m), 1 }, "Macro " + juce::String (m),
            juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.5f));

    const juce::StringArray waves { "Sine", "Triangle", "Saw", "Square", "S&H", "Smooth" };
    const juce::StringArray divs { "1/16", "1/8", "1/4", "1/2", "1", "2" };

    for (int i = 1; i <= 2; ++i)
    {
        const auto n = juce::String (i);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "lfo" + n + "Rate", 1 }, "LFO" + n + " Rate",
            juce::NormalisableRange<float> { 0.05f, 30.0f, 0.01f, 0.4f }, i == 1 ? 2.0f : 0.5f));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { "lfo" + n + "Wave", 1 }, "LFO" + n + " Wave", waves, 0));
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { "lfo" + n + "Sync", 1 }, "LFO" + n + " Sync", false));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { "lfo" + n + "Division", 1 }, "LFO" + n + " Div", divs, 2));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "lfo" + n + "ToCutoff", 1 }, "LFO" + n + "→Cutoff",
            juce::NormalisableRange<float> { -1.0f, 1.0f, 0.001f }, i == 1 ? 0.25f : 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "lfo" + n + "ToWt", 1 }, "LFO" + n + "→WT",
            juce::NormalisableRange<float> { -1.0f, 1.0f, 0.001f }, 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "lfo" + n + "ToPan", 1 }, "LFO" + n + "→Pan",
            juce::NormalisableRange<float> { -1.0f, 1.0f, 0.001f }, 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "lfo" + n + "ToAmp", 1 }, "LFO" + n + "→Amp",
            juce::NormalisableRange<float> { -1.0f, 1.0f, 0.001f }, 0.0f));
    }

    return { params.begin(), params.end() };
}

void ScorionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    engine.prepare (spec);
    setLatencySamples (0);
}

void ScorionAudioProcessor::releaseResources()
{
    engine.reset();
}

bool ScorionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
        return false;
    return true;
}

void ScorionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Merge on-screen / computer-keyboard notes into the host MIDI stream
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    engine.process (buffer, midi, apvts);
}

double ScorionAudioProcessor::getTailLengthSeconds() const
{
    return 3.0;
}

juce::AudioProcessorEditor* ScorionAudioProcessor::createEditor()
{
    return new ScorionAudioProcessorEditor (*this);
}

void ScorionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto root = juce::ValueTree ("ScorionState");
    root.appendChild (apvts.copyState(), nullptr);
    root.appendChild (uiSettings.createCopy(), nullptr);
    if (auto xml = root.createXml())
        copyXmlToBinary (*xml, destData);
}

void ScorionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr) return;

    if (xml->hasTagName ("ScorionState"))
    {
        auto root = juce::ValueTree::fromXml (*xml);
        auto params = root.getChildWithName (apvts.state.getType());
        if (params.isValid())
            apvts.replaceState (params);
        auto ui = root.getChildWithName ("UISettings");
        if (ui.isValid())
            uiSettings = ui;
        return;
    }

    // Legacy APVTS-only state
    if (xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

int ScorionAudioProcessor::getNumPrograms()
{
    // FL Studio / hosts: expose factory presets as programs (cap for host sanity)
    int n = 0;
    for (const auto& p : presetManager.presets())
        if (p.kind == LibraryItemKind::Preset)
            ++n;
    return juce::jmax (1, juce::jmin (n, 128));
}

int ScorionAudioProcessor::getCurrentProgram()
{
    int prog = 0;
    const int cur = presetManager.getCurrentIndex();
    for (int i = 0; i < (int) presetManager.presets().size() && i <= cur; ++i)
        if (presetManager.presets()[(size_t) i].kind == LibraryItemKind::Preset)
        {
            if (i == cur) return juce::jlimit (0, getNumPrograms() - 1, prog);
            ++prog;
        }
    return 0;
}

void ScorionAudioProcessor::setCurrentProgram (int index)
{
    int prog = 0;
    for (int i = 0; i < (int) presetManager.presets().size(); ++i)
    {
        if (presetManager.presets()[(size_t) i].kind != LibraryItemKind::Preset)
            continue;
        if (prog == index)
        {
            juce::ValueTree extra ("extra");
            presetManager.loadPreset (i, apvts, extra);
            return;
        }
        ++prog;
    }
}

const juce::String ScorionAudioProcessor::getProgramName (int index)
{
    int prog = 0;
    for (const auto& p : presetManager.presets())
    {
        if (p.kind != LibraryItemKind::Preset) continue;
        if (prog == index) return p.name;
        ++prog;
    }
    return "Init";
}

void ScorionAudioProcessor::panic()
{
    keyboardState.allNotesOff (0);
    engine.panic();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScorionAudioProcessor();
}
