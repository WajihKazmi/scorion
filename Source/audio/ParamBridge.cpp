#include "audio/ParamBridge.h"

void ParamBridge::prepare (double sampleRate)
{
    masterSmooth.prepare (sampleRate, 0.02f);
    cutoffSmooth.prepare (sampleRate, 0.01f);
    resSmooth.prepare (sampleRate, 0.01f);
    attackSmooth.prepare (sampleRate, 0.05f);
    releaseSmooth.prepare (sampleRate, 0.05f);
    reverbSmooth.prepare (sampleRate, 0.05f);
    delaySmooth.prepare (sampleRate, 0.05f);
}

void ParamBridge::snapshot (juce::AudioProcessorValueTreeState& apvts, VoiceRenderParams& out) noexcept
{
    auto load = [&] (const char* id, float fallback) {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return fallback;
    };

    cutoffSmooth.setTarget (load ("filterCutoff", 6000.0f));
    resSmooth.setTarget (load ("filterResonance", 0.2f));
    attackSmooth.setTarget (load ("ampAttack", 0.01f));
    releaseSmooth.setTarget (load ("ampRelease", 0.35f));
    masterSmooth.setTarget (load ("masterGain", 0.75f));
    reverbSmooth.setTarget (load ("reverbMix", 0.18f));
    delaySmooth.setTarget (load ("delayMix", 0.12f));

    // Advance smoothers a block-worth for a stable snapshot (simple approach)
    for (int i = 0; i < 32; ++i)
    {
        cutoffSmooth.next();
        resSmooth.next();
        attackSmooth.next();
        releaseSmooth.next();
        masterSmooth.next();
        reverbSmooth.next();
        delaySmooth.next();
    }

    out.cutoff = cutoffSmooth.getCurrent();
    out.resonance = resSmooth.getCurrent();
    out.attack = attackSmooth.getCurrent();
    out.decay = load ("ampDecay", 0.25f);
    out.sustain = load ("ampSustain", 0.7f);
    out.release = releaseSmooth.getCurrent();
    out.filterEnvAmount = load ("filterEnvAmount", 0.35f);
    out.pulseWidth = load ("pulseWidth", 0.5f);
    out.unisonDetune = load ("unisonDetune", 0.15f);
    out.unisonCount = (int) load ("unisonCount", 1.0f);
    out.wtPosition = load ("wtPosition", 0.0f);
    out.fmRatio = load ("fmRatio", 2.0f);
    out.fmIndex = load ("fmIndex", 1.5f);
    out.grainPosition = load ("grainPosition", 0.2f);
    out.grainSize = load ("grainSize", 0.08f);
    out.grainDensity = load ("grainDensity", 0.45f);
    out.masterGain = masterSmooth.getCurrent();

    masterGain = out.masterGain;
    reverbMix = reverbSmooth.getCurrent();
    delayMix = delaySmooth.getCurrent();
    engineIndex = (int) load ("engine", 0.0f);
}
