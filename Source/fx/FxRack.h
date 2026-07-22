#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>
#include "dsp/Oversampler2x.h"
#include "audio/PerformanceMode.h"

class FxRack
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setPerformanceMode (PerformanceMode mode) noexcept { perfMode_ = mode; }
    void process (juce::AudioBuffer<float>& buffer, float reverbMix, float delayMix, float masterGain);

private:
    void processEq (juce::AudioBuffer<float>& buffer);
    void processChorus (juce::AudioBuffer<float>& buffer);
    void processSaturator (juce::AudioBuffer<float>& buffer);
    void processDelay (juce::AudioBuffer<float>& buffer, float mix);
    void processReverb (juce::AudioBuffer<float>& buffer, float mix);
    void processLimiter (juce::AudioBuffer<float>& buffer);

    PerformanceMode perfMode_ = PerformanceMode::Eco;
    double sampleRate_ = 44100.0;
    int maxBlock_ = 512;

    std::vector<float> delayL_, delayR_;
    int delayWrite_ = 0;
    float delayTimeSamples_ = 24000.0f;
    float delayFeedback_ = 0.42f;
    float delayHpL_ = 0, delayHpR_ = 0;

    // 8-line FDN capacity; Eco only walks first 4
    static constexpr int kFdn = 8;
    std::array<std::vector<float>, kFdn> fdn_;
    std::array<int, kFdn> fdnIdx_ {};
    std::array<float, kFdn> fdnLp_ {};
    float preDelayL_ = 0, preDelayR_ = 0;
    std::vector<float> preL_, preR_;
    int preWrite_ = 0;

    std::vector<float> chorusL_, chorusR_;
    int chorusWrite_ = 0;
    float chorusPhase_ = 0.0f;

    Oversampler2x osL_, osR_;
    std::vector<float> satScratch_;
    float limEnv_ = 1.0f;
    float eqLpL_ = 0, eqLpR_ = 0;
};
