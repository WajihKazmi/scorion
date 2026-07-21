#pragma once

#include "ISynthEngine.h"
#include <array>
#include <vector>
#include <cmath>

class WavetableEngine final : public ISynthEngine
{
public:
    void prepare (double sampleRate, int maxBlock) override;
    void reset() override;
    void noteOn (VoiceId id, const NoteEvent& note) override;
    void noteOff (VoiceId id, const NoteEvent& note) override;
    void render (VoiceId id, float* left, float* right, int numSamples,
                 const ModulatedParams& params) override;
    SynthEngineType type() const noexcept override { return SynthEngineType::Wavetable; }

    void setTable (const std::vector<float>& framesInterleaved, int frameSize, int numFrames);
    bool loadWtBin (const void* data, size_t bytes);

private:
    struct VoiceState
    {
        double phase = 0.0;
        float baseHz = 440.0f;
        bool active = false;
    };

    static constexpr int kMax = 16;
    static constexpr int kMaxMips = 8;
    std::array<VoiceState, kMax> voices {};
    std::vector<float> table_;       // base frames
    std::vector<float> mips_;        // mip * frames * frameSize
    int frameSize_ = 2048;
    int numFrames_ = 256;
    int numMips_ = 1;
    double sampleRate_ = 44100.0;

    void buildBuiltinTable();
    void rebuildMips();
    float lookup (float framePos, double phase, float hz) const noexcept;
};
