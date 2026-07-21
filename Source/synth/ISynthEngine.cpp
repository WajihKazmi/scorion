#include "synth/ISynthEngine.h"
#include "synth/VirtualAnalogEngine.h"
#include "synth/WavetableEngine.h"
#include "synth/FmEngine.h"
#include "synth/GranularEngine.h"
#include "synth/SamplerEngine.h"

std::unique_ptr<ISynthEngine> createSynthEngine (SynthEngineType type)
{
    switch (type)
    {
        case SynthEngineType::VirtualAnalog: return std::make_unique<VirtualAnalogEngine>();
        case SynthEngineType::Wavetable:     return std::make_unique<WavetableEngine>();
        case SynthEngineType::FM:            return std::make_unique<FmEngine>();
        case SynthEngineType::Granular:      return std::make_unique<GranularEngine>();
        case SynthEngineType::Sampler:       return std::make_unique<SamplerEngine>();
        case SynthEngineType::Count:         break;
    }
    return std::make_unique<VirtualAnalogEngine>();
}
