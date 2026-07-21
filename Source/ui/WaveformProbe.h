#pragma once

#include <array>
#include <atomic>
#include <cmath>

/** Audio thread writes downsampled waveform; UI reads snapshot. */
class WaveformProbe
{
public:
    static constexpr int kSize = 256;

    void prepare() noexcept
    {
        writePos_.store (0);
        for (auto& s : buffer_) s = 0.0f;
    }

    void pushSample (float s) noexcept
    {
        accum_ += s;
        ++count_;
        if (count_ >= decim_)
        {
            const auto i = writePos_.load (std::memory_order_relaxed);
            buffer_[(size_t) i] = accum_ / (float) count_;
            writePos_.store ((i + 1) % kSize, std::memory_order_release);
            accum_ = 0.0f;
            count_ = 0;
        }
    }

    void setDecimation (int d) noexcept { decim_ = d > 0 ? d : 1; }

    void copySnapshot (std::array<float, kSize>& out) const noexcept
    {
        const auto start = writePos_.load (std::memory_order_acquire);
        for (int i = 0; i < kSize; ++i)
            out[(size_t) i] = buffer_[(size_t) ((start + i) % kSize)];
    }

private:
    std::array<float, kSize> buffer_ {};
    std::atomic<int> writePos_ { 0 };
    float accum_ = 0.0f;
    int count_ = 0;
    int decim_ = 8;
};
