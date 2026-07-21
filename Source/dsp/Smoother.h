#pragma once

#include <cmath>
#include <algorithm>

class Smoother
{
public:
    void prepare (double sampleRate, float timeSeconds = 0.01f) noexcept
    {
        sampleRate_ = sampleRate;
        setTime (timeSeconds);
        current_ = target_;
    }

    void setTime (float timeSeconds) noexcept
    {
        const float n = std::max (1.0f, (float) (timeSeconds * sampleRate_));
        coeff_ = 1.0f - std::exp (-1.0f / n);
    }

    void setTarget (float t) noexcept { target_ = t; }
    void setCurrentAndTarget (float v) noexcept { current_ = target_ = v; }

    float next() noexcept
    {
        current_ += coeff_ * (target_ - current_);
        return current_;
    }

    float getCurrent() const noexcept { return current_; }

private:
    double sampleRate_ = 44100.0;
    float coeff_ = 0.1f;
    float current_ = 0.0f;
    float target_ = 0.0f;
};
