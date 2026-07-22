#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include "synth/ISynthEngine.h"
#include "dsp/AdsrEnvelope.h"
#include "dsp/ZdfFilter.h"
#include "dsp/Oversampler2x.h"
#include "audio/PerformanceMode.h"

class ModulationMatrix;
class AssetLoader;

struct Voice
{
    bool active = false;
    bool sustaining = false;
    int note = 60;
    int channel = 1;
    float velocity = 0.8f;
    float mpePitch = 0.0f;
    float mpePressure = 0.0f;
    float mpeTimbre = 0.5f;
    uint32_t age = 0;
    AdsrEnvelope ampEnv;
    AdsrEnvelope filterEnv;
    ZdfFilter filter;
    MoogLadderFilter ladder;
    std::array<float, 8> unisonPhases {};
};

class VoiceManager
{
public:
    static constexpr int kMaxVoices = 16;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setPerformanceMode (PerformanceMode mode) noexcept { perfMode_ = mode; }
    PerformanceMode getPerformanceMode() const noexcept { return perfMode_; }
    void setEngine (SynthEngineType type, AssetLoader& assets);
    /** Hot-swap mono WAV into Sampler/Granular engines (sound-design hub). */
    bool loadSampleFromFile (const juce::File& wavFile, int rootNote = 60);
    bool loadWavetableFromFile (const juce::File& wtbinFile);
    SynthEngineType getEngineType() const noexcept { return engineType; }

    void noteOn (int note, float velocity, int channel, float mpePitch, float mpePressure, float mpeTimbre);
    void noteOff (int note, int channel);
    void allNotesOff();
    void setSustain (bool on);

    void process (juce::AudioBuffer<float>& buffer, int numSamples,
                  ModulationMatrix& mod, const VoiceRenderParams& baseParams);

    ISynthEngine* engine() noexcept { return currentEngine.get(); }

    int getActiveVoiceCount() const noexcept
    {
        int n = 0;
        for (const auto& v : voices)
            if (v.active) ++n;
        return n;
    }

private:
    int findFreeOrSteal();
    void renderVoice (Voice& voice, float* left, float* right, int numSamples,
                      ModulationMatrix& mod, const VoiceRenderParams& baseParams);

    std::array<Voice, kMaxVoices> voices {};
    std::unique_ptr<ISynthEngine> currentEngine;
    SynthEngineType engineType = SynthEngineType::VirtualAnalog;
    PerformanceMode perfMode_ = PerformanceMode::Eco;
    double sampleRate = 44100.0;
    bool sustainPedal = false;
    uint32_t ageCounter = 0;
};
