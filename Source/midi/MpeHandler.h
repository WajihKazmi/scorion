#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

struct MpeFrame
{
    float pitchBend = 0.0f;   // semitones
    float pressure = 0.0f;    // 0..1
    float timbre = 0.5f;      // 0..1
};

class MpeHandler
{
public:
    void reset();
    void processMidi (const juce::MidiMessage& msg, int channel);

    MpeFrame get (int channel) const noexcept;
    bool isMpeMemberChannel (int channel) const noexcept;

    int masterChannel = 1;
    int memberChannels = 15; // lower zone default

private:
    struct ChannelState
    {
        float pitchBend = 0.0f;
        float pressure = 0.0f;
        float timbre = 0.5f;
        int note = -1;
    };
    std::array<ChannelState, 17> channels_ {};
    float globalPitchRange_ = 48.0f;
};
