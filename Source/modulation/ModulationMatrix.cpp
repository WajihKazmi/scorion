#include "modulation/ModulationMatrix.h"
#include <cmath>

void ModulationMatrix::prepare (double sampleRate)
{
    sampleRate_ = sampleRate;
    for (auto& e : envs_) e.prepare (sampleRate);
    for (auto& l : lfos_) l.prepare (sampleRate);
    macros_.fill (0.5f);
    clearConnections();
}

void ModulationMatrix::reset()
{
    for (auto& e : envs_) e.reset();
    for (auto& l : lfos_) l.reset();
}

void ModulationMatrix::noteOn (int voice, float velocity, int note)
{
    if (voice < 0 || voice >= 16) return;
    velocities_[(size_t) voice] = velocity;
    keytracks_[(size_t) voice] = (note - 60) / 64.0f;
    randoms_[(size_t) voice] = (float) std::rand() / (float) RAND_MAX * 2.0f - 1.0f;
    envs_[0].noteOn();
    envs_[1].noteOn();
}

void ModulationMatrix::noteOff (int voice)
{
    (void) voice;
    envs_[0].noteOff();
    envs_[1].noteOff();
}

void ModulationMatrix::setMacro (int index, float value) noexcept
{
    if (index >= 0 && index < 8)
        macros_[(size_t) index] = value;
}

float ModulationMatrix::getMacro (int index) const noexcept
{
    if (index >= 0 && index < 8)
        return macros_[(size_t) index];
    return 0.0f;
}

bool ModulationMatrix::addConnection (ModSource src, ModDest dest, float depth, bool bipolar)
{
    for (auto& c : connections_)
    {
        if (! c.active)
        {
            c.active = true;
            c.source = src;
            c.dest = dest;
            c.depth = depth;
            c.bipolar = bipolar;
            return true;
        }
    }
    return false;
}

void ModulationMatrix::clearConnections()
{
    for (auto& c : connections_)
        c.active = false;
}

float ModulationMatrix::sourceValue (int voice, ModSource src) const noexcept
{
    switch (src)
    {
        case ModSource::Env1: return envs_[0].getLevel();
        case ModSource::Env2: return envs_[1].getLevel();
        case ModSource::Env3: return envs_[2].getLevel();
        case ModSource::Env4: return envs_[3].getLevel();
        case ModSource::Lfo1: case ModSource::Lfo2: case ModSource::Lfo3: case ModSource::Lfo4:
            return 0.0f; // filled in tick
        case ModSource::Macro1: case ModSource::Macro2: case ModSource::Macro3: case ModSource::Macro4:
        case ModSource::Macro5: case ModSource::Macro6: case ModSource::Macro7: case ModSource::Macro8:
            return macros_[(size_t) src - (size_t) ModSource::Macro1] * 2.0f - 1.0f;
        case ModSource::Velocity: return voice >= 0 && voice < 16 ? velocities_[(size_t) voice] : 0.0f;
        case ModSource::Keytrack: return voice >= 0 && voice < 16 ? keytracks_[(size_t) voice] : 0.0f;
        case ModSource::ModWheel: return modWheel_ * 2.0f - 1.0f;
        case ModSource::Aftertouch: return aftertouch_;
        case ModSource::MpePressure: return 0.0f;
        case ModSource::MpeTimbre: return 0.0f;
        case ModSource::Random1: case ModSource::Random2:
            return voice >= 0 && voice < 16 ? randoms_[(size_t) voice] : 0.0f;
        case ModSource::Count:
            break;
    }
    return 0.0f;
}

void ModulationMatrix::tickVoice (int voice, VoiceMod& out) noexcept
{
    out = {};
    // Advance shared LFOs once per call site expectation: advance on voice 0 only
    static thread_local float lfoCache[4] {};
    if (voice == 0)
    {
        for (int i = 0; i < 4; ++i)
            lfoCache[i] = lfos_[(size_t) i].next (bpm);
        envs_[0].next();
        envs_[1].next();
        envs_[2].next();
        envs_[3].next();
    }

    auto srcVal = [&] (ModSource s) -> float {
        if (s >= ModSource::Lfo1 && s <= ModSource::Lfo4)
            return lfoCache[(size_t) s - (size_t) ModSource::Lfo1];
        return sourceValue (voice, s);
    };

    for (auto& c : connections_)
    {
        if (! c.active) continue;
        float v = srcVal (c.source);
        if (! c.bipolar)
            v = 0.5f * (v + 1.0f);
        v *= c.depth;
        switch (c.dest)
        {
            case ModDest::Pitch: out.pitch += v * 12.0f; break;
            case ModDest::Amp: out.amp *= (1.0f + v); break;
            case ModDest::Cutoff: out.cutoff += v; break;
            case ModDest::Resonance: out.resonance += v * 0.5f; break;
            case ModDest::PulseWidth: out.pulseWidth += v; break;
            case ModDest::WtPosition: out.wtPosition += v; break;
            case ModDest::FmIndex: out.fmIndex += v; break;
            case ModDest::GrainPos: out.grainPos += v; break;
            case ModDest::GrainDensity: out.grainDensity += v; break;
            case ModDest::Pan: out.pan += v; break;
            case ModDest::Count: break;
        }
    }

    out.amp = std::clamp (out.amp, 0.0f, 2.0f);
}
