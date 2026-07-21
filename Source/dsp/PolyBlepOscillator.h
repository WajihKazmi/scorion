#pragma once

#include <cmath>
#include <algorithm>

enum class OscWaveform { Sine, Saw, Square, Triangle };

/** Band-limited oscillator using PolyBLEP for classic VA waveforms. */
class PolyBlepOscillator
{
public:
    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        phase_ = 0.0;
    }

    void setFrequency (float hz) noexcept
    {
        frequency_ = std::max (0.0f, hz);
        phaseInc_ = (double) frequency_ / sampleRate_;
    }

    void setWaveform (OscWaveform w) noexcept { waveform_ = w; }
    void setPulseWidth (float pw) noexcept { pulseWidth_ = std::clamp (pw, 0.05f, 0.95f); }
    void resetPhase() noexcept { phase_ = 0.0; }

    float next() noexcept
    {
        float value = 0.0f;
        switch (waveform_)
        {
            case OscWaveform::Sine:
                value = std::sin ((float) (phase_ * juceTwoPi));
                break;
            case OscWaveform::Saw:
                value = (float) (2.0 * phase_ - 1.0);
                value -= polyBlep (phase_);
                break;
            case OscWaveform::Square:
            {
                value = phase_ < pulseWidth_ ? 1.0f : -1.0f;
                value += polyBlep (phase_);
                value -= polyBlep (std::fmod (phase_ + (1.0 - pulseWidth_), 1.0));
                break;
            }
            case OscWaveform::Triangle:
            {
                // Integrate bandlimited square approximation
                float sq = phase_ < 0.5 ? 1.0f : -1.0f;
                sq += polyBlep (phase_);
                sq -= polyBlep (std::fmod (phase_ + 0.5, 1.0));
                value = lastTri_ + (float) phaseInc_ * sq * 2.0f;
                lastTri_ = value * 0.995f;
                value *= 0.8f;
                break;
            }
        }

        phase_ += phaseInc_;
        if (phase_ >= 1.0)
            phase_ -= 1.0;
        return value;
    }

private:
    static constexpr float juceTwoPi = 6.28318530717958647692f;

    float polyBlep (double t) const noexcept
    {
        const double dt = phaseInc_;
        if (t < dt)
        {
            t /= dt;
            return (float) (t + t - t * t - 1.0);
        }
        if (t > 1.0 - dt)
        {
            t = (t - 1.0) / dt;
            return (float) (t * t + t + t + 1.0);
        }
        return 0.0f;
    }

    double sampleRate_ = 44100.0;
    double phase_ = 0.0;
    double phaseInc_ = 0.0;
    float frequency_ = 440.0f;
    float pulseWidth_ = 0.5f;
    float lastTri_ = 0.0f;
    OscWaveform waveform_ = OscWaveform::Saw;
};
