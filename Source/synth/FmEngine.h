#pragma once

#include "ISynthEngine.h"
#include <array>
#include <cmath>

class FmEngine final : public ISynthEngine
{
public:
    void prepare (double sampleRate, int maxBlock) override;
    void reset() override;
    void noteOn (VoiceId id, const NoteEvent& note) override;
    void noteOff (VoiceId id, const NoteEvent& note) override;
    void render (VoiceId id, float* left, float* right, int numSamples,
                 const ModulatedParams& params) override;
    SynthEngineType type() const noexcept override { return SynthEngineType::FM; }

private:
    struct Op { double phase = 0.0; };
    struct VoiceState
    {
        std::array<Op, 4> ops {};
        float baseHz = 440.0f;
        bool active = false;
    };

    static constexpr int kMax = 16;
    std::array<VoiceState, kMax> voices {};
    double sampleRate_ = 44100.0;
};
