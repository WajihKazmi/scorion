#include "synth/GranularEngine.h"
#include <juce_core/juce_core.h>
#include <cmath>

void GranularEngine::ensureBuiltinSample()
{
    if (! sample_.empty()) return;
    sample_.resize (44100);
    for (size_t i = 0; i < sample_.size(); ++i)
    {
        const float t = (float) i / 44100.0f;
        sample_[i] = 0.5f * std::sin (2.0f * 3.14159265f * 220.0f * t)
                   + 0.25f * std::sin (2.0f * 3.14159265f * 440.0f * t)
                   + 0.1f * (((float) (i % 97) / 97.0f) - 0.5f);
    }
    fileSampleRate_ = 44100.0;
}

void GranularEngine::prepare (double sampleRate, int)
{
    sampleRate_ = sampleRate;
    ensureBuiltinSample();
}

void GranularEngine::setSample (const std::vector<float>& mono, double sampleRate)
{
    if (mono.empty())
    {
        ensureBuiltinSample();
        return;
    }
    sample_ = mono;
    fileSampleRate_ = sampleRate;
}

void GranularEngine::reset()
{
    for (auto& v : voices)
    {
        v.active = false;
        for (auto& g : v.grains) g.active = false;
    }
}

void GranularEngine::noteOn (VoiceId id, const NoteEvent& note)
{
    if (id < 0 || id >= kMaxVoices) return;
    auto& v = voices[(size_t) id];
    v.active = true;
    v.baseHz = 440.0f * std::pow (2.0f, (note.note - 69) / 12.0f);
    v.spawnAccum = 0.0f;
    for (auto& g : v.grains) g.active = false;
}

void GranularEngine::noteOff (VoiceId, const NoteEvent&) {}

void GranularEngine::spawnGrain (VoiceState& v, const ModulatedParams& params)
{
    for (auto& g : v.grains)
    {
        if (g.active) continue;
        g.active = true;
        const float spray = ((float) std::rand() / (float) RAND_MAX - 0.5f) * 0.05f;
        const double start = juce::jlimit (0.0, (double) sample_.size() - 2.0,
                                           (params.grainPosition + spray) * (double) sample_.size());
        g.pos = start;
        const float pitchRatio = (v.baseHz * std::pow (2.0f, params.pitchSemitones / 12.0f)) / 440.0f;
        g.increment = pitchRatio * (fileSampleRate_ / sampleRate_);
        g.length = juce::jmax (16, (int) (params.grainSize * sampleRate_));
        g.age = 0;
        g.pan = 0.5f + ((float) std::rand() / (float) RAND_MAX - 0.5f) * 0.4f;
        g.amp = 0.6f;
        return;
    }
    // steal oldest
    int oldest = 0;
    int maxAge = -1;
    for (int i = 0; i < kMaxGrains; ++i)
        if (v.grains[(size_t) i].age > maxAge)
        {
            maxAge = v.grains[(size_t) i].age;
            oldest = i;
        }
    v.grains[(size_t) oldest].active = false;
    spawnGrain (v, params);
}

void GranularEngine::render (VoiceId id, float* left, float* right, int numSamples,
                             const ModulatedParams& params)
{
    if (id < 0 || id >= kMaxVoices) return;
    auto& v = voices[(size_t) id];
    if (! v.active || sample_.empty()) return;

    const float spawnRate = 20.0f + params.grainDensity * 80.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        v.spawnAccum += spawnRate / (float) sampleRate_;
        while (v.spawnAccum >= 1.0f)
        {
            v.spawnAccum -= 1.0f;
            spawnGrain (v, params);
        }

        float l = 0.0f, r = 0.0f;
        for (auto& g : v.grains)
        {
            if (! g.active) continue;
            const float env = 0.5f - 0.5f * std::cos (6.28318530718f * (float) g.age / (float) g.length);
            const int ip = juce::jlimit (0, (int) sample_.size() - 1, (int) g.pos);
            const float s = sample_[(size_t) ip] * env * g.amp;
            l += s * (1.0f - g.pan);
            r += s * g.pan;
            g.pos += g.increment;
            ++g.age;
            if (g.age >= g.length || g.pos >= (double) sample_.size())
                g.active = false;
        }
        left[i] = l * 0.4f;
        right[i] = r * 0.4f;
    }
}
