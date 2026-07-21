#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "VoiceManager.h"
#include "ParamBridge.h"
#include "SpscQueue.h"
#include "modulation/ModulationMatrix.h"
#include "fx/FxRack.h"
#include "midi/MpeHandler.h"
#include "ui/WaveformProbe.h"
#include "assets/AssetLoader.h"

class AudioEngine
{
public:
    AudioEngine();

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi,
                  juce::AudioProcessorValueTreeState& apvts);

    void panic();

    VoiceManager& voices() noexcept { return voiceManager; }
    ModulationMatrix& modulation() noexcept { return modMatrix; }
    FxRack& fx() noexcept { return fxRack; }
    ParamBridge& params() noexcept { return paramBridge; }
    WaveformProbe& probe() noexcept { return waveformProbe; }
    AssetLoader& assets() noexcept { return assetLoader; }
    MpeHandler& mpe() noexcept { return mpeHandler; }

private:
    ParamBridge paramBridge;
    ModulationMatrix modMatrix;
    VoiceManager voiceManager;
    FxRack fxRack;
    MpeHandler mpeHandler;
    WaveformProbe waveformProbe;
    AssetLoader assetLoader;
    juce::AudioBuffer<float> voiceBuffer;
    double sampleRate = 44100.0;
};
