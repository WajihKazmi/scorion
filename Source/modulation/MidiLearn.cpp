#include "modulation/MidiLearn.h"

void MidiLearn::clear() noexcept
{
    const juce::ScopedLock sl (lock_);
    for (auto& m : maps_)
    {
        m.cc = -1;
        m.paramId = {};
    }
}

void MidiLearn::arm (const juce::String& paramId) noexcept
{
    armedParam_ = paramId;
    armed_.store (true);
}

void MidiLearn::learnCc (int cc, const juce::String& paramId)
{
    const juce::ScopedLock sl (lock_);
    for (auto& m : maps_)
    {
        if (m.cc == cc || m.cc < 0)
        {
            m.cc = cc;
            m.paramId = paramId.isNotEmpty() ? paramId : armedParam_;
            armed_.store (false);
            return;
        }
    }
}

bool MidiLearn::getParamForCc (int cc, juce::String& outId) const
{
    const juce::ScopedLock sl (lock_);
    for (const auto& m : maps_)
        if (m.cc == cc)
        {
            outId = m.paramId;
            return true;
        }
    return false;
}

juce::ValueTree MidiLearn::toValueTree() const
{
    juce::ValueTree tree ("MidiLearn");
    const juce::ScopedLock sl (lock_);
    for (const auto& m : maps_)
    {
        if (m.cc < 0) continue;
        juce::ValueTree node ("Map");
        node.setProperty ("cc", m.cc, nullptr);
        node.setProperty ("param", m.paramId, nullptr);
        tree.appendChild (node, nullptr);
    }
    return tree;
}

void MidiLearn::fromValueTree (const juce::ValueTree& tree)
{
    clear();
    const juce::ScopedLock sl (lock_);
    int i = 0;
    for (int c = 0; c < tree.getNumChildren() && i < kMaxMaps; ++c)
    {
        auto node = tree.getChild (c);
        maps_[(size_t) i].cc = (int) node.getProperty ("cc", -1);
        maps_[(size_t) i].paramId = node.getProperty ("param").toString();
        ++i;
    }
}
