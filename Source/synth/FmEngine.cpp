#include "synth/FmEngine.h"
#include <cmath>

void FmEngine::prepare (double sampleRate, int)
{
    sampleRate_ = sampleRate;
}

void FmEngine::reset()
{
    for (auto& v : voices) v.active = false;
}

void FmEngine::noteOn (VoiceId id, const NoteEvent& note)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    v.active = true;
    v.baseHz = 440.0f * std::pow (2.0f, (note.note - 69) / 12.0f);
    for (auto& op : v.ops) op.phase = 0.0;
}

void FmEngine::noteOff (VoiceId, const NoteEvent&) {}

void FmEngine::render (VoiceId id, float* left, float* right, int numSamples,
                       const ModulatedParams& params)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    if (! v.active) return;

    const float hz = v.baseHz * std::pow (2.0f, params.pitchSemitones / 12.0f);
    const float ratio = params.fmRatio;
    const float index = params.fmIndex;

    // Simple 2-op style: op2 modulates op1, with mild feedback
    for (int i = 0; i < numSamples; ++i)
    {
        const float mod = std::sin ((float) (v.ops[1].phase * 6.28318530718));
        const float car = std::sin ((float) (v.ops[0].phase * 6.28318530718) + mod * index);
        const float op3 = std::sin ((float) (v.ops[2].phase * 6.28318530718));
        const float out = 0.7f * car + 0.3f * op3;

        left[i] = out;
        right[i] = out;

        v.ops[0].phase += (double) hz / sampleRate_;
        v.ops[1].phase += (double) (hz * ratio) / sampleRate_;
        v.ops[2].phase += (double) (hz * ratio * 0.5f) / sampleRate_;
        v.ops[3].phase += (double) (hz * 3.0f) / sampleRate_;
        for (auto& op : v.ops)
            if (op.phase >= 1.0) op.phase -= std::floor (op.phase);
    }
}
