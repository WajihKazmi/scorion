#pragma once

#include "ISynthEngine.h"
#include <array>
#include <vector>

class SamplerEngine final : public ISynthEngine
{
public:
    void prepare (double sampleRate, int maxBlock) override;
    void reset() override;
    void noteOn (VoiceId id, const NoteEvent& note) override;
    void noteOff (VoiceId id, const NoteEvent& note) override;
    void render (VoiceId id, float* left, float* right, int numSamples,
                 const ModulatedParams& params) override;
    SynthEngineType type() const noexcept override { return SynthEngineType::Sampler; }

    void setSample (const std::vector<float>& mono, double sampleRate, int rootNote = 60);

private:
    struct VoiceState
    {
        bool active = false;
        double pos = 0.0;
        float baseHz = 440.0f;
        int rootNote = 60;
    };

    static constexpr int kMax = 16;
    std::array<VoiceState, kMax> voices {};
    std::vector<float> sample_;
    double fileSampleRate_ = 44100.0;
    double sampleRate_ = 44100.0;
    int rootNote_ = 60;
    bool loop_ = true;
    int loopStart_ = 0;
    int loopEnd_ = 0;

    void ensureBuiltinSample();
    float readHermite (double pos) const noexcept;
};
