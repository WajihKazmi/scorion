#include "audio/AudioEngine.h"
#include "synth/ISynthEngine.h"
#include "modulation/Lfo.h"

AudioEngine::AudioEngine() = default;

void AudioEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    paramBridge.prepare (spec.sampleRate);
    modMatrix.prepare (spec.sampleRate);
    voiceManager.prepare (spec);
    fxRack.prepare (spec);
    mpeHandler.reset();
    waveformProbe.prepare();
    waveformProbe.setDecimation (juce::jmax (1, (int) (spec.sampleRate / 8000.0)));
    voiceBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize, false, false, true);
    voiceManager.setEngine (SynthEngineType::VirtualAnalog, assetLoader);
}

void AudioEngine::reset()
{
    voiceManager.reset();
    fxRack.reset();
    modMatrix.reset();
}

void AudioEngine::panic()
{
    voiceManager.allNotesOff();
    modMatrix.reset();
}

namespace
{
float divisionBeats (int index)
{
    static constexpr float table[] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };
    return table[juce::jlimit (0, 5, index)];
}

LfoWave waveFromIndex (int index)
{
    switch (juce::jlimit (0, 5, index))
    {
        case 1: return LfoWave::Triangle;
        case 2: return LfoWave::Saw;
        case 3: return LfoWave::Square;
        case 4: return LfoWave::SampleHold;
        case 5: return LfoWave::SmoothRandom;
        default: return LfoWave::Sine;
    }
}

void wireLfo (ModulationMatrix& mod, juce::AudioProcessorValueTreeState& apvts, int idx, ModSource src)
{
    const auto n = juce::String (idx + 1);
    auto* rate = apvts.getRawParameterValue ("lfo" + n + "Rate");
    auto* wave = apvts.getRawParameterValue ("lfo" + n + "Wave");
    auto* sync = apvts.getRawParameterValue ("lfo" + n + "Sync");
    auto* div = apvts.getRawParameterValue ("lfo" + n + "Division");
    if (rate == nullptr) return;

    auto& lfo = mod.lfo (idx);
    lfo.setRateHz (rate->load());
    if (wave != nullptr)
        lfo.setWave (waveFromIndex ((int) wave->load()));
    const bool synced = sync != nullptr && sync->load() > 0.5f;
    const int divIdx = div != nullptr ? (int) div->load() : 2;
    lfo.setTempoSync (synced, divisionBeats (divIdx));

    auto add = [&] (const char* suffix, ModDest dest) {
        if (auto* p = apvts.getRawParameterValue ("lfo" + n + suffix))
        {
            const float d = p->load();
            if (std::abs (d) > 0.001f)
                mod.addConnection (src, dest, d, true);
        }
    };
    add ("ToCutoff", ModDest::Cutoff);
    add ("ToWt", ModDest::WtPosition);
    add ("ToPan", ModDest::Pan);
    add ("ToAmp", ModDest::Amp);
}
} // namespace

void AudioEngine::process (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi,
                           juce::AudioProcessorValueTreeState& apvts)
{
    VoiceRenderParams base;
    paramBridge.snapshot (apvts, base);

    const int engineIdx = (int) apvts.getRawParameterValue ("engine")->load();
    const auto wanted = (SynthEngineType) juce::jlimit (0, (int) SynthEngineType::Count - 1, engineIdx);
    if (wanted != voiceManager.getEngineType())
        voiceManager.setEngine (wanted, assetLoader);

    modMatrix.bpm = 120.0f;
    modMatrix.clearConnections();
    wireLfo (modMatrix, apvts, 0, ModSource::Lfo1);
    wireLfo (modMatrix, apvts, 1, ModSource::Lfo2);

    for (int i = 0; i < 8; ++i)
    {
        auto id = "macro" + juce::String (i + 1);
        if (auto* p = apvts.getRawParameterValue (id))
            modMatrix.setMacro (i, p->load());
    }

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int ch = msg.getChannel();

        if (msg.isController() && msg.getControllerNumber() == 64)
            voiceManager.setSustain (msg.getControllerValue() >= 64);
        if (msg.isController() && msg.getControllerNumber() == 1)
            modMatrix.setModWheel (msg.getControllerValue() / 127.0f);
        if (msg.isChannelPressure())
            modMatrix.setAftertouch (msg.getChannelPressureValue() / 127.0f);

        mpeHandler.processMidi (msg, ch);
        const auto mpe = mpeHandler.get (ch);

        if (msg.isNoteOn())
        {
            voiceManager.noteOn (msg.getNoteNumber(), msg.getFloatVelocity(), ch,
                                 mpe.pitchBend, mpe.pressure, mpe.timbre);
        }
        else if (msg.isNoteOff())
        {
            voiceManager.noteOff (msg.getNoteNumber(), ch);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            panic();
        }
    }

    buffer.clear();
    voiceBuffer.clear();
    voiceManager.process (voiceBuffer, buffer.getNumSamples(), modMatrix, base);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, voiceBuffer, juce::jmin (ch, voiceBuffer.getNumChannels() - 1), 0, buffer.getNumSamples());

    fxRack.process (buffer, paramBridge.reverbMix, paramBridge.delayMix, paramBridge.masterGain);

    if (buffer.getNumChannels() > 0)
    {
        const float* r = buffer.getReadPointer (0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            waveformProbe.pushSample (r[i]);
    }
}
