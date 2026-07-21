#include "synth/SamplerEngine.h"
#include <juce_core/juce_core.h>
#include <cmath>

void SamplerEngine::ensureBuiltinSample()
{
    if (! sample_.empty()) return;
    // Soft pluck-like decaying harmonic series
    sample_.resize (44100);
    for (size_t i = 0; i < sample_.size(); ++i)
    {
        const float t = (float) i / 44100.0f;
        const float env = std::exp (-3.0f * t);
        float s = 0.0f;
        for (int h = 1; h <= 6; ++h)
            s += std::sin (2.0f * 3.14159265f * 220.0f * (float) h * t) / (float) h;
        sample_[i] = s * env * 0.4f;
    }
    fileSampleRate_ = 44100.0;
    loopStart_ = 8000;
    loopEnd_ = 40000;
    rootNote_ = 57; // A3 ~ 220
}

void SamplerEngine::prepare (double sampleRate, int)
{
    sampleRate_ = sampleRate;
    ensureBuiltinSample();
}

void SamplerEngine::setSample (const std::vector<float>& mono, double sampleRate, int rootNote)
{
    if (mono.empty())
    {
        ensureBuiltinSample();
        return;
    }
    sample_ = mono;
    fileSampleRate_ = sampleRate;
    rootNote_ = rootNote;
    loopStart_ = (int) (sample_.size() * 0.15);
    loopEnd_ = (int) (sample_.size() * 0.9);
}

void SamplerEngine::reset()
{
    for (auto& v : voices) v.active = false;
}

void SamplerEngine::noteOn (VoiceId id, const NoteEvent& note)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    v.active = true;
    v.pos = 0.0;
    v.baseHz = 440.0f * std::pow (2.0f, (note.note - 69) / 12.0f);
    v.rootNote = rootNote_;
}

void SamplerEngine::noteOff (VoiceId, const NoteEvent&) {}

float SamplerEngine::readHermite (double pos) const noexcept
{
    const int n = (int) sample_.size();
    if (n < 4) return 0.0f;
    int i1 = (int) pos;
    const float x = (float) (pos - (double) i1);
    auto at = [&] (int i) {
        while (i < 0) i += n;
        while (i >= n) i -= n;
        return sample_[(size_t) i];
    };
    const float y0 = at (i1 - 1);
    const float y1 = at (i1);
    const float y2 = at (i1 + 1);
    const float y3 = at (i1 + 2);
    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * x + c2) * x + c1) * x + c0;
}

void SamplerEngine::render (VoiceId id, float* left, float* right, int numSamples,
                            const ModulatedParams& params)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    if (! v.active || sample_.empty()) return;

    const float noteHz = v.baseHz * std::pow (2.0f, params.pitchSemitones / 12.0f);
    const float rootHz = 440.0f * std::pow (2.0f, (rootNote_ - 69) / 12.0f);
    const double inc = (noteHz / rootHz) * (fileSampleRate_ / sampleRate_);

    for (int i = 0; i < numSamples; ++i)
    {
        if (v.pos >= (double) sample_.size())
        {
            if (loop_)
                v.pos = (double) loopStart_;
            else
            {
                left[i] = right[i] = 0.0f;
                continue;
            }
        }
        if (loop_ && v.pos >= (double) loopEnd_)
            v.pos = (double) loopStart_;

        const float s = readHermite (v.pos);
        left[i] = s;
        right[i] = s;
        v.pos += inc;
    }
}
