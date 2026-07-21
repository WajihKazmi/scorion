#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/Smoother.h"
#include "synth/ISynthEngine.h"

class ParamBridge
{
public:
    void prepare (double sampleRate);
    void snapshot (juce::AudioProcessorValueTreeState& apvts, VoiceRenderParams& out) noexcept;

    float masterGain = 0.8f;
    float reverbMix = 0.15f;
    float delayMix = 0.1f;
    int engineIndex = 0;

private:
    Smoother masterSmooth;
    Smoother cutoffSmooth;
    Smoother resSmooth;
    Smoother attackSmooth;
    Smoother releaseSmooth;
    Smoother reverbSmooth;
    Smoother delaySmooth;
};
