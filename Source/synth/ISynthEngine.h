#pragma once

#include <cstdint>
#include <memory>

enum class SynthEngineType : int
{
    VirtualAnalog = 0,
    Wavetable,
    FM,
    Granular,
    Sampler,
    Count
};

inline const char* synthEngineName (SynthEngineType t)
{
    switch (t)
    {
        case SynthEngineType::VirtualAnalog: return "Virtual Analog";
        case SynthEngineType::Wavetable:     return "Wavetable";
        case SynthEngineType::FM:            return "FM";
        case SynthEngineType::Granular:      return "Granular";
        case SynthEngineType::Sampler:       return "Sampler";
        case SynthEngineType::Count:         break;
    }
    return "Unknown";
}

using VoiceId = int;

struct NoteEvent
{
    int note = 60;
    float velocity = 0.8f;
    int channel = 1;
    float mpePitch = 0.0f;
    float mpePressure = 0.0f;
    float mpeTimbre = 0.5f;
};

struct VoiceRenderParams
{
    float cutoff = 8000.0f;
    float resonance = 0.2f;
    float attack = 0.01f;
    float decay = 0.2f;
    float sustain = 0.7f;
    float release = 0.3f;
    float filterEnvAmount = 0.4f;
    float oscMix = 1.0f;
    float pulseWidth = 0.5f;
    float unisonDetune = 0.1f;
    int unisonCount = 1;
    float wtPosition = 0.0f;
    float fmRatio = 2.0f;
    float fmIndex = 1.0f;
    float grainPosition = 0.25f;
    float grainSize = 0.08f;
    float grainDensity = 0.5f;
    float sampleStart = 0.0f;
    float masterGain = 0.8f;
};

struct ModulatedParams : VoiceRenderParams
{
    float pitchSemitones = 0.0f;
    float ampMod = 1.0f;
};

class ISynthEngine
{
public:
    virtual ~ISynthEngine() = default;
    virtual void prepare (double sampleRate, int maxBlock) = 0;
    virtual void reset() = 0;
    virtual void noteOn (VoiceId id, const NoteEvent& note) = 0;
    virtual void noteOff (VoiceId id, const NoteEvent& note) = 0;
    virtual void render (VoiceId id, float* left, float* right, int numSamples,
                         const ModulatedParams& params) = 0;
    virtual SynthEngineType type() const noexcept = 0;
};

std::unique_ptr<ISynthEngine> createSynthEngine (SynthEngineType type);
