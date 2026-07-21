#include "fx/FxRack.h"
#include <juce_core/juce_core.h>
#include <cmath>

namespace
{
constexpr int kFdnSizes[8] = { 1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116 };
}

void FxRack::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    maxBlock_ = (int) spec.maximumBlockSize;

    const int delaySize = (int) (sampleRate_ * 2.5) + maxBlock_ + 8;
    delayL_.assign ((size_t) delaySize, 0.0f);
    delayR_.assign ((size_t) delaySize, 0.0f);
    delayWrite_ = 0;
    delayTimeSamples_ = (float) (sampleRate_ * 0.42);

    for (int i = 0; i < kFdn; ++i)
    {
        const int n = (int) ((double) kFdnSizes[i] * (sampleRate_ / 44100.0));
        fdn_[(size_t) i].assign ((size_t) juce::jmax (64, n), 0.0f);
        fdnIdx_[(size_t) i] = 0;
        fdnLp_[(size_t) i] = 0.0f;
    }

    const int preN = (int) (sampleRate_ * 0.03) + 8;
    preL_.assign ((size_t) preN, 0.0f);
    preR_.assign ((size_t) preN, 0.0f);
    preWrite_ = 0;

    const int chN = (int) (sampleRate_ * 0.05) + 8;
    chorusL_.assign ((size_t) chN, 0.0f);
    chorusR_.assign ((size_t) chN, 0.0f);
    chorusWrite_ = 0;
    chorusPhase_ = 0.0f;

    osL_.prepare (maxBlock_);
    osR_.prepare (maxBlock_);
    satScratch_.assign ((size_t) maxBlock_ * 2 + 16, 0.0f);
    limEnv_ = 1.0f;
}

void FxRack::reset()
{
    std::fill (delayL_.begin(), delayL_.end(), 0.0f);
    std::fill (delayR_.begin(), delayR_.end(), 0.0f);
    for (auto& d : fdn_) std::fill (d.begin(), d.end(), 0.0f);
    std::fill (preL_.begin(), preL_.end(), 0.0f);
    std::fill (preR_.begin(), preR_.end(), 0.0f);
    std::fill (chorusL_.begin(), chorusL_.end(), 0.0f);
    std::fill (chorusR_.begin(), chorusR_.end(), 0.0f);
    osL_.reset();
    osR_.reset();
    limEnv_ = 1.0f;
}

void FxRack::processEq (juce::AudioBuffer<float>& buffer)
{
    // Musical tilt + subtle air shelf
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float l = buffer.getSample (0, i);
        float r = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : l;
        eqLpL_ += 0.04f * (l - eqLpL_);
        eqLpR_ += 0.04f * (r - eqLpR_);
        const float highL = l - eqLpL_;
        const float highR = r - eqLpR_;
        l = eqLpL_ * 1.05f + highL * 1.18f;
        r = eqLpR_ * 1.05f + highR * 1.18f;
        buffer.setSample (0, i, l);
        if (buffer.getNumChannels() > 1)
            buffer.setSample (1, i, r);
    }
}

void FxRack::processChorus (juce::AudioBuffer<float>& buffer)
{
    if (chorusL_.empty()) return;
    const int size = (int) chorusL_.size();
    const float depth = (float) (sampleRate_ * 0.0035);
    const float base = (float) (sampleRate_ * 0.012);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        chorusPhase_ += (0.25f * juce::MathConstants<float>::twoPi) / (float) sampleRate_;
        if (chorusPhase_ > juce::MathConstants<float>::twoPi)
            chorusPhase_ -= juce::MathConstants<float>::twoPi;

        const float modL = base + depth * std::sin (chorusPhase_);
        const float modR = base + depth * std::sin (chorusPhase_ + 1.7f);
        const float inL = buffer.getSample (0, i);
        const float inR = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : inL;

        chorusL_[(size_t) chorusWrite_] = inL;
        chorusR_[(size_t) chorusWrite_] = inR;

        auto readAt = [&] (const std::vector<float>& buf, float delaySamples) {
            float read = (float) chorusWrite_ - delaySamples;
            while (read < 0.0f) read += (float) size;
            const int i0 = (int) read;
            const int i1 = (i0 + 1) % size;
            const float frac = read - (float) i0;
            return buf[(size_t) (i0 % size)] * (1.0f - frac) + buf[(size_t) i1] * frac;
        };

        const float wetL = readAt (chorusL_, modL);
        const float wetR = readAt (chorusR_, modR);
        chorusWrite_ = (chorusWrite_ + 1) % size;

        buffer.setSample (0, i, inL * 0.78f + wetL * 0.22f);
        if (buffer.getNumChannels() > 1)
            buffer.setSample (1, i, inR * 0.78f + wetR * 0.22f);
    }
}

void FxRack::processSaturator (juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    auto processChan = [&] (int ch, Oversampler2x& os) {
        auto* data = buffer.getWritePointer (ch);
        float* up = os.upsample (data, n);
        for (int i = 0; i < n * 2; ++i)
        {
            // Multi-stage soft clip (tape-ish)
            float x = up[i] * 1.35f;
            x = std::tanh (x);
            x = x - 0.15f * x * x * x;
            up[i] = x;
        }
        os.downsample (up, data, n);
    };
    processChan (0, osL_);
    if (buffer.getNumChannels() > 1)
        processChan (1, osR_);
}

void FxRack::processDelay (juce::AudioBuffer<float>& buffer, float mix)
{
    if (delayL_.empty() || mix <= 0.0001f) return;
    const int size = (int) delayL_.size();
    const int readOffset = (int) delayTimeSamples_;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const int read = (delayWrite_ - readOffset + size * 8) % size;
        float dl = delayL_[(size_t) read];
        float dr = delayR_[(size_t) read];
        float inL = buffer.getSample (0, i);
        float inR = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : inL;

        // Feedback HP to avoid low-end mud (soundscape friendly)
        float fbL = inL + dr * delayFeedback_;
        float fbR = inR + dl * delayFeedback_;
        delayHpL_ += 0.02f * (fbL - delayHpL_);
        delayHpR_ += 0.02f * (fbR - delayHpR_);
        delayL_[(size_t) delayWrite_] = fbL - delayHpL_ * 0.85f;
        delayR_[(size_t) delayWrite_] = fbR - delayHpR_ * 0.85f;
        delayWrite_ = (delayWrite_ + 1) % size;

        buffer.setSample (0, i, inL + dl * mix);
        if (buffer.getNumChannels() > 1)
            buffer.setSample (1, i, inR + dr * mix);
    }
}

void FxRack::processReverb (juce::AudioBuffer<float>& buffer, float mix)
{
    if (mix <= 0.0001f || fdn_[0].empty() || preL_.empty()) return;
    const float feedback = 0.88f;
    const float damp = 0.28f;
    const int preN = (int) preL_.size();

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float inL = buffer.getSample (0, i);
        float inR = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : inL;

        preL_[(size_t) preWrite_] = inL;
        preR_[(size_t) preWrite_] = inR;
        const int preRead = (preWrite_ + 1) % preN;
        const float pdL = preL_[(size_t) preRead];
        const float pdR = preR_[(size_t) preRead];
        preWrite_ = (preWrite_ + 1) % preN;

        // Householder-ish FDN mix
        std::array<float, kFdn> taps {};
        float sum = 0.0f;
        for (int c = 0; c < kFdn; ++c)
        {
            auto& buf = fdn_[(size_t) c];
            int& idx = fdnIdx_[(size_t) c];
            float& lp = fdnLp_[(size_t) c];
            float t = buf[(size_t) idx];
            lp += damp * (t - lp);
            taps[(size_t) c] = lp;
            sum += lp;
        }

        const float input = 0.5f * (pdL + pdR);
        for (int c = 0; c < kFdn; ++c)
        {
            auto& buf = fdn_[(size_t) c];
            int& idx = fdnIdx_[(size_t) c];
            // diffuse feedback: self - mean
            const float fb = (taps[(size_t) c] - 0.125f * sum) * feedback;
            const float inj = (c & 1) ? pdR : pdL;
            buf[(size_t) idx] = inj * 0.35f + input * 0.15f + fb;
            idx = (idx + 1) % (int) buf.size();
        }

        float wetL = 0.0f, wetR = 0.0f;
        for (int c = 0; c < kFdn; ++c)
        {
            if (c & 1) wetR += taps[(size_t) c];
            else wetL += taps[(size_t) c];
        }
        wetL *= 0.22f;
        wetR *= 0.22f;

        const float dry = 1.0f - mix * 0.85f;
        buffer.setSample (0, i, inL * dry + wetL * mix);
        if (buffer.getNumChannels() > 1)
            buffer.setSample (1, i, inR * dry + wetR * mix);
    }
}

void FxRack::processLimiter (juce::AudioBuffer<float>& buffer)
{
    const float ceiling = 0.97f;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            peak = juce::jmax (peak, std::abs (buffer.getSample (ch, i)));

        const float target = peak > ceiling ? ceiling / (peak + 1.0e-6f) : 1.0f;
        const float coeff = target < limEnv_ ? 0.08f : 0.004f; // fast attack, slow release
        limEnv_ += coeff * (target - limEnv_);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample (ch, i, buffer.getSample (ch, i) * limEnv_);
    }
}

void FxRack::process (juce::AudioBuffer<float>& buffer, float reverbMix, float delayMix, float masterGain)
{
    processEq (buffer);
    processChorus (buffer);
    processSaturator (buffer);
    processDelay (buffer, delayMix);
    processReverb (buffer, reverbMix);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain (ch, 0, buffer.getNumSamples(), masterGain * 0.92f);

    processLimiter (buffer);
}
