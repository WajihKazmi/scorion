#pragma once

#include "ISynthEngine.h"
#include <array>
#include <vector>

class GranularEngine final : public ISynthEngine
{
public:
    void prepare (double sampleRate, int maxBlock) override;
    void reset() override;
    void noteOn (VoiceId id, const NoteEvent& note) override;
    void noteOff (VoiceId id, const NoteEvent& note) override;
    void render (VoiceId id, float* left, float* right, int numSamples,
                 const ModulatedParams& params) override;
    SynthEngineType type() const noexcept override { return SynthEngineType::Granular; }

    void setSample (const std::vector<float>& mono, double sampleRate);

private:
    static constexpr int kMaxVoices = 16;
    static constexpr int kMaxGrains = 64;

    struct Grain
    {
        bool active = false;
        double pos = 0.0;
        double increment = 1.0;
        int age = 0;
        int length = 1;
        float pan = 0.5f;
        float amp = 1.0f;
    };

    struct VoiceState
    {
        bool active = false;
        float baseHz = 440.0f;
        float spawnAccum = 0.0f;
        std::array<Grain, kMaxGrains> grains {};
    };

    std::array<VoiceState, kMaxVoices> voices {};
    std::vector<float> sample_;
    double fileSampleRate_ = 44100.0;
    double sampleRate_ = 44100.0;

    void ensureBuiltinSample();
    void spawnGrain (VoiceState& v, const ModulatedParams& params);
};
