#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <vector>

/** Lightweight 2x polyphase half-band style oversampler for saturators / drive. */
class Oversampler2x
{
public:
    void prepare (int maxBlock)
    {
        upBuffer_.assign ((size_t) maxBlock * 2 + 8, 0.0f);
        downBuffer_.assign ((size_t) maxBlock + 8, 0.0f);
        z1_ = z2_ = z3_ = z4_ = 0.0f;
    }

    void reset() noexcept { z1_ = z2_ = z3_ = z4_ = 0.0f; }

    /** Upsample mono block -> 2x length in upBuffer_. Returns pointer + count. */
    float* upsample (const float* in, int n) noexcept
    {
        // Zero-stuff + cheap lowpass (binomial)
        for (int i = 0; i < n; ++i)
        {
            const float x = in[i];
            // even
            float y = 0.5f * (x + z1_);
            z1_ = x;
            upBuffer_[(size_t) (i * 2)] = y;
            // odd interpolated
            upBuffer_[(size_t) (i * 2 + 1)] = 0.5f * (x + y);
        }
        return upBuffer_.data();
    }

    /** Downsample 2x mono -> out[n] */
    void downsample (const float* in2x, float* out, int n) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const float a = in2x[i * 2];
            const float b = in2x[i * 2 + 1];
            // simple average + one-pole smooth
            float y = 0.5f * (a + b);
            z2_ += 0.5f * (y - z2_);
            out[i] = z2_;
        }
    }

    float* upData() noexcept { return upBuffer_.data(); }

private:
    std::vector<float> upBuffer_, downBuffer_;
    float z1_ = 0, z2_ = 0, z3_ = 0, z4_ = 0;
};

/** Moog-style 4-pole ladder with tanh nonlinearities (stable musical default). */
class MoogLadderFilter
{
public:
    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept
    {
        s1_ = s2_ = s3_ = s4_ = 0.0f;
    }

    void setCutoff (float hz) noexcept
    {
        cutoff_ = std::clamp (hz, 20.0f, (float) sampleRate_ * 0.42f);
    }

    void setResonance (float res) noexcept
    {
        resonance_ = std::clamp (res, 0.0f, 0.95f) * 4.0f;
    }

    float process (float in) noexcept
    {
        const float f = std::clamp (cutoff_ * 2.0f / (float) sampleRate_, 0.0f, 1.0f);
        const float g = 1.0f - std::exp (-6.0f * f);

        float x = in - resonance_ * s4_;
        x = std::tanh (x);

        s1_ += g * (std::tanh (x) - std::tanh (s1_));
        s2_ += g * (std::tanh (s1_) - std::tanh (s2_));
        s3_ += g * (std::tanh (s2_) - std::tanh (s3_));
        s4_ += g * (std::tanh (s3_) - std::tanh (s4_));
        return s4_;
    }

private:
    double sampleRate_ = 44100.0;
    float cutoff_ = 8000.0f;
    float resonance_ = 0.0f;
    float s1_ = 0, s2_ = 0, s3_ = 0, s4_ = 0;
};
