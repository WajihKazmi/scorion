#include "midi/MpeHandler.h"
#include <array>

void MpeHandler::reset()
{
    channels_ = {};
}

bool MpeHandler::isMpeMemberChannel (int channel) const noexcept
{
    return channel > masterChannel && channel <= masterChannel + memberChannels;
}

void MpeHandler::processMidi (const juce::MidiMessage& msg, int channel)
{
    if (channel < 1 || channel > 16) return;
    auto& st = channels_[(size_t) channel];

    if (msg.isPitchWheel())
    {
        const float norm = (msg.getPitchWheelValue() - 8192) / 8192.0f;
        st.pitchBend = norm * (isMpeMemberChannel (channel) ? globalPitchRange_ : 2.0f);
    }
    else if (msg.isChannelPressure())
    {
        st.pressure = msg.getChannelPressureValue() / 127.0f;
    }
    else if (msg.isController() && msg.getControllerNumber() == 74)
    {
        st.timbre = msg.getControllerValue() / 127.0f;
    }
    else if (msg.isNoteOn())
    {
        st.note = msg.getNoteNumber();
    }
    else if (msg.isNoteOff() && st.note == msg.getNoteNumber())
    {
        st.note = -1;
    }
}

MpeFrame MpeHandler::get (int channel) const noexcept
{
    if (channel < 1 || channel > 16) return {};
    const auto& st = channels_[(size_t) channel];
    return { st.pitchBend, st.pressure, st.timbre };
}
