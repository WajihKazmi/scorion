#include "synth/WavetableEngine.h"
#include <juce_core/juce_core.h>
#include <cstring>

void WavetableEngine::prepare (double sampleRate, int)
{
    sampleRate_ = sampleRate;
    if (table_.empty())
        buildBuiltinTable();
    if (mips_.empty())
        rebuildMips();
}

void WavetableEngine::buildBuiltinTable()
{
    frameSize_ = 2048;
    numFrames_ = 64;
    table_.assign ((size_t) (frameSize_ * numFrames_), 0.0f);
    for (int f = 0; f < numFrames_; ++f)
    {
        const int harmonics = 2 + f;
        for (int i = 0; i < frameSize_; ++i)
        {
            float s = 0.0f;
            const float phase = (float) i / (float) frameSize_;
            for (int h = 1; h <= harmonics; ++h)
                s += std::sin (phase * 6.28318530718f * (float) h) / (float) h;
            table_[(size_t) (f * frameSize_ + i)] = s * 0.45f;
        }
    }
    rebuildMips();
}

void WavetableEngine::rebuildMips()
{
    if (table_.empty() || frameSize_ <= 0 || numFrames_ <= 0)
        return;

    numMips_ = 1;
    int fs = frameSize_;
    while (fs > 64 && numMips_ < kMaxMips)
    {
        fs /= 2;
        ++numMips_;
    }

    mips_.assign ((size_t) numMips_ * (size_t) numFrames_ * (size_t) frameSize_, 0.0f);

    // mip 0 = original
    std::memcpy (mips_.data(), table_.data(), table_.size() * sizeof (float));

    // Build lower mips by harmonic-limiting via FFT-free box spectral approx:
    // successive half-band average + upsample back to frameSize (bandlimit effect)
    std::vector<float> work (table_);
    for (int mip = 1; mip < numMips_; ++mip)
    {
        for (int f = 0; f < numFrames_; ++f)
        {
            const float* src = work.data() + (size_t) f * (size_t) frameSize_;
            float* dst = mips_.data() + ((size_t) mip * (size_t) numFrames_ + (size_t) f) * (size_t) frameSize_;

            // 4-tap lowpass then copy (progressively darker)
            for (int i = 0; i < frameSize_; ++i)
            {
                const int i0 = (i - 1 + frameSize_) % frameSize_;
                const int i1 = i;
                const int i2 = (i + 1) % frameSize_;
                const int i3 = (i + 2) % frameSize_;
                dst[i] = 0.25f * (src[i0] + src[i1] + src[i2] + src[i3]);
            }
        }
        // feed next mip from this result
        for (int f = 0; f < numFrames_; ++f)
        {
            const float* src = mips_.data() + ((size_t) mip * (size_t) numFrames_ + (size_t) f) * (size_t) frameSize_;
            float* dst = work.data() + (size_t) f * (size_t) frameSize_;
            std::memcpy (dst, src, (size_t) frameSize_ * sizeof (float));
        }
    }
}

void WavetableEngine::setTable (const std::vector<float>& framesInterleaved, int frameSize, int numFrames)
{
    if (framesInterleaved.empty() || frameSize <= 0 || numFrames <= 0)
    {
        buildBuiltinTable();
        return;
    }
    frameSize_ = frameSize;
    numFrames_ = numFrames;
    table_ = framesInterleaved;
    rebuildMips();
}

bool WavetableEngine::loadWtBin (const void* data, size_t bytes)
{
    if (data == nullptr || bytes < 8)
        return false;
    const auto* p = static_cast<const uint8_t*> (data);
    int frameSize = 0, numFrames = 0;
    std::memcpy (&frameSize, p, 4);
    std::memcpy (&numFrames, p + 4, 4);
    if (frameSize <= 0 || numFrames <= 0)
        return false;
    const size_t need = 8 + (size_t) frameSize * (size_t) numFrames * sizeof (float);
    if (bytes < need)
        return false;
    std::vector<float> table ((size_t) frameSize * (size_t) numFrames);
    std::memcpy (table.data(), p + 8, table.size() * sizeof (float));
    setTable (table, frameSize, numFrames);
    return true;
}

void WavetableEngine::reset()
{
    for (auto& v : voices) v.active = false;
}

void WavetableEngine::noteOn (VoiceId id, const NoteEvent& note)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    v.active = true;
    v.phase = 0.0;
    v.baseHz = 440.0f * std::pow (2.0f, (note.note - 69) / 12.0f);
}

void WavetableEngine::noteOff (VoiceId, const NoteEvent&) {}

float WavetableEngine::lookup (float framePos, double phase, float hz) const noexcept
{
    if (mips_.empty()) return 0.0f;

    // Choose mip so that ~max harmonic stays under Nyquist
    const float cyclesPerSample = hz / (float) sampleRate_;
    int mip = 0;
    while (mip + 1 < numMips_ && cyclesPerSample * (float) (frameSize_ >> mip) > 0.45f)
        ++mip;

    const float fPos = juce::jlimit (0.0f, (float) numFrames_ - 1.001f, framePos * (float) (numFrames_ - 1));
    const int f0 = (int) fPos;
    const int f1 = juce::jmin (f0 + 1, numFrames_ - 1);
    const float fracF = fPos - (float) f0;
    const double idx = phase * (double) frameSize_;
    const int i0 = (int) idx;
    const int i1 = (i0 + 1) % frameSize_;
    const float frac = (float) (idx - (double) i0);

    const size_t base = (size_t) mip * (size_t) numFrames_ * (size_t) frameSize_;
    const float a0 = mips_[base + (size_t) (f0 * frameSize_ + i0)];
    const float a1 = mips_[base + (size_t) (f0 * frameSize_ + i1)];
    const float b0 = mips_[base + (size_t) (f1 * frameSize_ + i0)];
    const float b1 = mips_[base + (size_t) (f1 * frameSize_ + i1)];
    // Hermite-ish blend between samples
    const float a = a0 + (a1 - a0) * frac;
    const float b = b0 + (b1 - b0) * frac;
    return a + (b - a) * fracF;
}

void WavetableEngine::render (VoiceId id, float* left, float* right, int numSamples,
                              const ModulatedParams& params)
{
    if (id < 0 || id >= kMax) return;
    auto& v = voices[(size_t) id];
    if (! v.active || mips_.empty()) return;

    const float hz = v.baseHz * std::pow (2.0f, params.pitchSemitones / 12.0f);
    const double inc = (double) hz / sampleRate_;
    const int unison = juce::jlimit (1, 7, params.unisonCount);

    for (int i = 0; i < numSamples; ++i)
    {
        float mixed = 0.0f;
        for (int u = 0; u < unison; ++u)
        {
            const float det = (unison == 1) ? 0.0f
                : ((float) u / (float) (unison - 1) - 0.5f) * 2.0f * params.unisonDetune * 0.08f;
            const float f = hz * (1.0f + det);
            const double phase = std::fmod (v.phase + (double) u * 0.07, 1.0);
            mixed += lookup (params.wtPosition, phase < 0 ? phase + 1.0 : phase, f);
        }
        mixed /= (float) unison;
        // Gentle soft clip — Serum-like glue
        mixed = std::tanh (mixed * 1.15f);
        left[i] = mixed;
        right[i] = mixed;
        v.phase += inc;
        if (v.phase >= 1.0) v.phase -= 1.0;
    }
}
