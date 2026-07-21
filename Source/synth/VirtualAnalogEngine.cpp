#include "synth/VirtualAnalogEngine.h"
#include <juce_core/juce_core.h>
#include <cmath>

void VirtualAnalogEngine::prepare (double sampleRate, int)
{
    sampleRate_ = sampleRate;
    for (auto& v : voices)
    {
        v.osc1.prepare (sampleRate);
        v.osc2.prepare (sampleRate);
        v.sub.prepare (sampleRate);
        v.osc1.setWaveform (OscWaveform::Saw);
        v.osc2.setWaveform (OscWaveform::Square);
        v.sub.setWaveform (OscWaveform::Sine);
    }
}

void VirtualAnalogEngine::reset()
{
    for (auto& v : voices)
        v.active = false;
}

void VirtualAnalogEngine::noteOn (VoiceId id, const NoteEvent& note)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    v.active = true;
    v.baseHz = 440.0f * std::pow (2.0f, (note.note - 69) / 12.0f);
    v.osc1.resetPhase();
    v.osc2.resetPhase();
    v.sub.resetPhase();
}

void VirtualAnalogEngine::noteOff (VoiceId id, const NoteEvent&)
{
    juce::ignoreUnused (id);
}

void VirtualAnalogEngine::render (VoiceId id, float* left, float* right, int numSamples,
                                  const ModulatedParams& params)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    if (! v.active) return;

    const float hz = v.baseHz * std::pow (2.0f, params.pitchSemitones / 12.0f);
    const int unison = juce::jlimit (1, 8, params.unisonCount);

    for (int i = 0; i < numSamples; ++i)
    {
        float mixed = 0.0f;
        for (int u = 0; u < unison; ++u)
        {
            const float det = (unison == 1) ? 0.0f
                : ((float) u / (float) (unison - 1) - 0.5f) * 2.0f * params.unisonDetune * 0.05f;
            const float f = hz * (1.0f + det);
            v.osc1.setFrequency (f);
            v.osc2.setFrequency (f * 1.005f);
            v.osc2.setPulseWidth (params.pulseWidth);
            v.sub.setFrequency (f * 0.5f);
            mixed += 0.55f * v.osc1.next() + 0.35f * v.osc2.next() + 0.25f * v.sub.next();
        }
        mixed /= (float) unison;
        left[i] = mixed;
        right[i] = mixed;
    }
}
