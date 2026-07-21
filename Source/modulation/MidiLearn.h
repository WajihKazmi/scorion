#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <juce_data_structures/juce_data_structures.h>

class MidiLearn
{
public:
    static constexpr int kMaxMaps = 128;

    void clear() noexcept;
    void arm (const juce::String& paramId) noexcept;
    bool isArmed() const noexcept { return armed_.load(); }
    void disarm() noexcept { armed_.store (false); }

    /** Call from message thread when CC received while armed. */
    void learnCc (int cc, const juce::String& paramId);

    /** Audio/message: resolve CC to parameter id index table. */
    bool getParamForCc (int cc, juce::String& outId) const;

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& tree);

private:
    struct Map { int cc = -1; juce::String paramId; };
    std::array<Map, kMaxMaps> maps_ {};
    std::atomic<bool> armed_ { false };
    juce::String armedParam_;
    mutable juce::CriticalSection lock_;
};
