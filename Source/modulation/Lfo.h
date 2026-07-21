#pragma once

#include <cmath>
#include <algorithm>

enum class LfoWave { Sine, Triangle, Saw, Square, SampleHold, SmoothRandom };

class Lfo
{
public:
    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        phase_ = 0.0;
    }

    void setRateHz (float hz) noexcept { rateHz_ = std::max (0.01f, hz); }
    void setWave (LfoWave w) noexcept { wave_ = w; }
    void setTempoSync (bool on, float divisionBeats = 1.0f) noexcept
    {
        tempoSync_ = on;
        divisionBeats_ = divisionBeats;
    }

    void reset() noexcept { phase_ = 0.0; }

    float next (float bpm = 120.0f) noexcept
    {
        const float rate = tempoSync_
            ? (bpm / 60.0f) / std::max (0.0625f, divisionBeats_)
            : rateHz_;
        const double inc = (double) rate / sampleRate_;
        float v = 0.0f;
        switch (wave_)
        {
            case LfoWave::Sine:     v = std::sin ((float) (phase_ * 6.28318530718)); break;
            case LfoWave::Triangle: v = phase_ < 0.5 ? (float) (4.0 * phase_ - 1.0)
                                                     : (float) (3.0 - 4.0 * phase_); break;
            case LfoWave::Saw:      v = (float) (2.0 * phase_ - 1.0); break;
            case LfoWave::Square:   v = phase_ < 0.5f ? 1.0f : -1.0f; break;
            case LfoWave::SampleHold:
                if (phase_ < inc)
                    held_ = (float) ((double) std::rand() / RAND_MAX) * 2.0f - 1.0f;
                v = held_;
                break;
            case LfoWave::SmoothRandom:
                if (phase_ < inc)
                    target_ = (float) ((double) std::rand() / RAND_MAX) * 2.0f - 1.0f;
                held_ += 0.001f * (target_ - held_);
                v = held_;
                break;
        }
        phase_ += inc;
        if (phase_ >= 1.0) phase_ -= 1.0;
        return v;
    }

private:
    double sampleRate_ = 44100.0;
    double phase_ = 0.0;
    float rateHz_ = 2.0f;
    float divisionBeats_ = 1.0f;
    float held_ = 0.0f;
    float target_ = 0.0f;
    bool tempoSync_ = false;
    LfoWave wave_ = LfoWave::Sine;
};
