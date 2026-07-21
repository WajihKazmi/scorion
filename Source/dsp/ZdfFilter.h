#pragma once

#include <cmath>
#include <algorithm>

enum class FilterType { Lowpass, Highpass, Bandpass, Notch };

/** Topology-preserving transform (TPT) ZDF state-variable filter — stable, musical. */
class ZdfFilter
{
public:
    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() noexcept { ic1eq_ = ic2eq_ = 0.0f; }
    void setType (FilterType t) noexcept { type_ = t; }

    void setCutoff (float hz) noexcept
    {
        cutoff_ = std::clamp (hz, 20.0f, (float) sampleRate_ * 0.45f);
    }

    void setResonance (float res) noexcept
    {
        // 0..1 -> q
        resonance_ = std::clamp (res, 0.0f, 0.98f);
    }

    float process (float x) noexcept
    {
        const float g = std::tan ((float) (3.14159265358979323846 * cutoff_ / sampleRate_));
        const float k = 2.0f - 2.0f * resonance_; // 0 res -> k=2 (butter), high res -> k small
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        const float v3 = x - ic2eq_;
        const float v1 = a1 * ic1eq_ + a2 * v3;
        const float v2 = ic2eq_ + a2 * ic1eq_ + a3 * v3;
        ic1eq_ = 2.0f * v1 - ic1eq_;
        ic2eq_ = 2.0f * v2 - ic2eq_;

        switch (type_)
        {
            case FilterType::Lowpass:  return v2;
            case FilterType::Highpass: return x - k * v1 - v2;
            case FilterType::Bandpass: return v1;
            case FilterType::Notch:    return x - k * v1;
        }
        return v2;
    }

private:
    double sampleRate_ = 44100.0;
    float cutoff_ = 8000.0f;
    float resonance_ = 0.2f;
    float ic1eq_ = 0.0f;
    float ic2eq_ = 0.0f;
    FilterType type_ = FilterType::Lowpass;
};
