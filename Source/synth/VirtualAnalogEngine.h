#pragma once

#include "ISynthEngine.h"
#include "dsp/PolyBlepOscillator.h"
#include <array>

class VirtualAnalogEngine final : public ISynthEngine
{
public:
    void prepare (double sampleRate, int maxBlock) override;
    void reset() override;
    void noteOn (VoiceId id, const NoteEvent& note) override;
    void noteOff (VoiceId id, const NoteEvent& note) override;
    void render (VoiceId id, float* left, float* right, int numSamples,
                 const ModulatedParams& params) override;
    SynthEngineType type() const noexcept override { return SynthEngineType::VirtualAnalog; }

private:
    struct VoiceOsc
    {
        PolyBlepOscillator osc1;
        PolyBlepOscillator osc2;
        PolyBlepOscillator sub;
        float baseHz = 440.0f;
        bool active = false;
    };

    static constexpr int kMax = 16;
    std::array<VoiceOsc, kMax> voices {};
    double sampleRate_ = 44100.0;
};
