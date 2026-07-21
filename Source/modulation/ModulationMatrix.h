#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "dsp/AdsrEnvelope.h"
#include "Lfo.h"

enum class ModSource : uint8_t
{
    Env1, Env2, Env3, Env4,
    Lfo1, Lfo2, Lfo3, Lfo4,
    Macro1, Macro2, Macro3, Macro4, Macro5, Macro6, Macro7, Macro8,
    Velocity, Keytrack, ModWheel, Aftertouch, MpePressure, MpeTimbre, Random1, Random2,
    Count
};

enum class ModDest : uint8_t
{
    Pitch, Amp, Cutoff, Resonance, PulseWidth, WtPosition, FmIndex, GrainPos, GrainDensity, Pan,
    Count
};

struct ModConnection
{
    bool active = false;
    ModSource source = ModSource::Lfo1;
    ModDest dest = ModDest::Cutoff;
    float depth = 0.0f;
    bool bipolar = true;
};

class ModulationMatrix
{
public:
    static constexpr int kMaxConnections = 64;

    void prepare (double sampleRate);
    void reset();

    void noteOn (int voice, float velocity, int note);
    void noteOff (int voice);
    AdsrEnvelope& env (int index) noexcept { return envs_[(size_t) index]; }
    Lfo& lfo (int index) noexcept { return lfos_[(size_t) index]; }

    void setMacro (int index, float value) noexcept;
    float getMacro (int index) const noexcept;

    bool addConnection (ModSource src, ModDest dest, float depth, bool bipolar = true);
    void clearConnections();
    const std::array<ModConnection, kMaxConnections>& connections() const noexcept { return connections_; }

    struct VoiceMod
    {
        float pitch = 0.0f;
        float amp = 1.0f;
        float cutoff = 0.0f;
        float resonance = 0.0f;
        float pulseWidth = 0.0f;
        float wtPosition = 0.0f;
        float fmIndex = 0.0f;
        float grainPos = 0.0f;
        float grainDensity = 0.0f;
        float pan = 0.0f;
    };

    void tickVoice (int voice, VoiceMod& out) noexcept;

    void setModWheel (float v) noexcept { modWheel_ = v; }
    void setAftertouch (float v) noexcept { aftertouch_ = v; }
    float bpm = 120.0f;

private:
    float sourceValue (int voice, ModSource src) const noexcept;

    std::array<AdsrEnvelope, 4> envs_ {};
    std::array<Lfo, 4> lfos_ {};
    std::array<float, 8> macros_ {};
    std::array<ModConnection, kMaxConnections> connections_ {};
    std::array<float, 16> velocities_ {};
    std::array<float, 16> keytracks_ {};
    std::array<float, 16> randoms_ {};
    float modWheel_ = 0.0f;
    float aftertouch_ = 0.0f;
    double sampleRate_ = 44100.0;
};
