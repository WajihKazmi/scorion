#pragma once

#include <array>
#include <atomic>
#include <cstdint>

/** Fixed-capacity lock-free SPSC queue for UI <-> audio messages. */
template <typename T, std::size_t Capacity>
class SpscQueue
{
public:
    static_assert ((Capacity & (Capacity - 1)) == 0, "Capacity must be power of two");

    bool push (const T& item) noexcept
    {
        const auto head = head_.load (std::memory_order_relaxed);
        const auto next = (head + 1) & (Capacity - 1);
        if (next == tail_.load (std::memory_order_acquire))
            return false;
        buffer_[head] = item;
        head_.store (next, std::memory_order_release);
        return true;
    }

    bool pop (T& item) noexcept
    {
        const auto tail = tail_.load (std::memory_order_relaxed);
        if (tail == head_.load (std::memory_order_acquire))
            return false;
        item = buffer_[tail];
        tail_.store ((tail + 1) & (Capacity - 1), std::memory_order_release);
        return true;
    }

    void clear() noexcept
    {
        tail_.store (head_.load (std::memory_order_relaxed), std::memory_order_relaxed);
    }

private:
    std::array<T, Capacity> buffer_ {};
    std::atomic<std::size_t> head_ { 0 };
    std::atomic<std::size_t> tail_ { 0 };
};

enum class UiToAudioMsg : uint8_t
{
    Panic = 1,
    LoadPresetIndex,
    SetEngine
};

struct UiMessage
{
    UiToAudioMsg type = UiToAudioMsg::Panic;
    int32_t value = 0;
};
