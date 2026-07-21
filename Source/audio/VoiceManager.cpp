#include "audio/VoiceManager.h"
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <cmath>
#include "modulation/ModulationMatrix.h"
#include "assets/AssetLoader.h"
#include "assets/ResourcePaths.h"
#include "synth/VirtualAnalogEngine.h"
#include "synth/WavetableEngine.h"
#include "synth/FmEngine.h"
#include "synth/GranularEngine.h"
#include "synth/SamplerEngine.h"

void VoiceManager::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    for (auto& v : voices)
    {
        v.ampEnv.prepare (sampleRate);
        v.filterEnv.prepare (sampleRate);
        v.filter.prepare (sampleRate);
        v.ladder.prepare (sampleRate);
    }
    if (currentEngine)
        currentEngine->prepare (sampleRate, (int) spec.maximumBlockSize);
}

void VoiceManager::reset()
{
    for (auto& v : voices)
    {
        v.active = false;
        v.ampEnv.reset();
        v.filterEnv.reset();
        v.filter.reset();
        v.ladder.reset();
    }
    if (currentEngine)
        currentEngine->reset();
}

void VoiceManager::setEngine (SynthEngineType type, AssetLoader& assets)
{
    engineType = type;
    currentEngine = createSynthEngine (type);
    if (currentEngine)
        currentEngine->prepare (sampleRate, 512);

    if (auto* wt = dynamic_cast<WavetableEngine*> (currentEngine.get()))
    {
        const auto repoTables = ResourcePaths::factoryWavetables();
        const juce::String preferred[] = {
            "formant_vox.wtbin", "choir_ah.wtbin", "talk_box.wtbin", "vocal_pad.wtbin",
            "glass_pad.wtbin", "harmonic_evolve.wtbin", "metallic.wtbin",
            "analog_warm.wtbin", "square_morph.wtbin", "digital_harsh.wtbin", "noise_bitcrush.wtbin"
        };
        bool loaded = false;
        for (auto& name : preferred)
        {
            auto f = repoTables.getChildFile (name);
            if (! f.existsAsFile()) continue;
            juce::MemoryBlock mb;
            if (f.loadFileAsData (mb) && wt->loadWtBin (mb.getData(), mb.getSize()))
            {
                loaded = true;
                break;
            }
        }
        if (! loaded)
        {
            const auto& a = assets.builtinWavetable();
            wt->setTable (a.samples, 2048, 4);
        }
    }
    if (auto* gr = dynamic_cast<GranularEngine*> (currentEngine.get()))
    {
        auto sampleDir = ResourcePaths::factorySamples();
        auto first = sampleDir.getChildFile ("vocal_ah_loop.wav");
        if (! first.existsAsFile())
            first = sampleDir.findChildFiles (juce::File::findFiles, false, "*.wav").getFirst();
        if (first.existsAsFile())
            loadSampleFromFile (first, 60);
        else
            gr->setSample (assets.builtinSample().samples, assets.builtinSample().sampleRate);
    }
    if (auto* sm = dynamic_cast<SamplerEngine*> (currentEngine.get()))
    {
        auto sampleDir = ResourcePaths::factorySamples();
        auto first = sampleDir.getChildFile ("keys_ep_soft.wav");
        if (! first.existsAsFile())
            first = sampleDir.findChildFiles (juce::File::findFiles, false, "*.wav").getFirst();
        if (first.existsAsFile())
            loadSampleFromFile (first, 60);
        else
            sm->setSample (assets.builtinSample().samples, assets.builtinSample().sampleRate,
                           assets.builtinSample().rootNote);
    }

    reset();
}

bool VoiceManager::loadSampleFromFile (const juce::File& wavFile, int rootNote)
{
    if (! wavFile.existsAsFile())
        return false;

    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (wavFile));
    if (reader == nullptr)
        return false;

    const int n = (int) juce::jmin ((juce::int64) 44100 * 12, reader->lengthInSamples);
    juce::AudioBuffer<float> buf ((int) reader->numChannels, n);
    reader->read (&buf, 0, n, 0, true, true);
    std::vector<float> mono ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        float s = buf.getSample (0, i);
        if (buf.getNumChannels() > 1)
            s = 0.5f * (s + buf.getSample (1, i));
        mono[(size_t) i] = s;
    }

    if (auto* sm = dynamic_cast<SamplerEngine*> (currentEngine.get()))
        sm->setSample (mono, reader->sampleRate, rootNote);
    if (auto* gr = dynamic_cast<GranularEngine*> (currentEngine.get()))
        gr->setSample (mono, reader->sampleRate);
    return true;
}

bool VoiceManager::loadWavetableFromFile (const juce::File& wtbinFile)
{
    if (! wtbinFile.existsAsFile())
        return false;
    if (auto* wt = dynamic_cast<WavetableEngine*> (currentEngine.get()))
    {
        juce::MemoryBlock mb;
        if (wtbinFile.loadFileAsData (mb) && wt->loadWtBin (mb.getData(), mb.getSize()))
            return true;
    }
    return false;
}

int VoiceManager::findFreeOrSteal()
{
    for (int i = 0; i < kMaxVoices; ++i)
        if (! voices[(size_t) i].active)
            return i;

    int best = 0;
    float bestScore = 1.0e9f;
    for (int i = 0; i < kMaxVoices; ++i)
    {
        const float score = voices[(size_t) i].ampEnv.getLevel() + (float) (ageCounter - voices[(size_t) i].age) * 0.00001f;
        if (score < bestScore)
        {
            bestScore = score;
            best = i;
        }
    }
    return best;
}

void VoiceManager::noteOn (int note, float velocity, int channel,
                           float mpePitch, float mpePressure, float mpeTimbre)
{
    const int idx = findFreeOrSteal();
    auto& v = voices[(size_t) idx];
    v.active = true;
    v.sustaining = false;
    v.note = note;
    v.channel = channel;
    v.velocity = velocity;
    v.mpePitch = mpePitch;
    v.mpePressure = mpePressure;
    v.mpeTimbre = mpeTimbre;
    v.age = ++ageCounter;
    v.ampEnv.noteOn();
    v.filterEnv.noteOn();
    v.filter.reset();

    NoteEvent ev;
    ev.note = note;
    ev.velocity = velocity;
    ev.channel = channel;
    ev.mpePitch = mpePitch;
    ev.mpePressure = mpePressure;
    ev.mpeTimbre = mpeTimbre;
    if (currentEngine)
        currentEngine->noteOn (idx, ev);
}

void VoiceManager::noteOff (int note, int channel)
{
    for (int i = 0; i < kMaxVoices; ++i)
    {
        auto& v = voices[(size_t) i];
        if (v.active && v.note == note && v.channel == channel)
        {
            if (sustainPedal)
            {
                v.sustaining = true;
            }
            else
            {
                v.ampEnv.noteOff();
                v.filterEnv.noteOff();
                if (currentEngine)
                    currentEngine->noteOff (i, NoteEvent { note, 0.0f, channel, 0, 0, 0 });
            }
        }
    }
}

void VoiceManager::setSustain (bool on)
{
    sustainPedal = on;
    if (! on)
    {
        for (int i = 0; i < kMaxVoices; ++i)
        {
            auto& v = voices[(size_t) i];
            if (v.sustaining)
            {
                v.sustaining = false;
                v.ampEnv.noteOff();
                v.filterEnv.noteOff();
            }
        }
    }
}

void VoiceManager::allNotesOff()
{
    for (int i = 0; i < kMaxVoices; ++i)
    {
        voices[(size_t) i].active = false;
        voices[(size_t) i].ampEnv.reset();
        voices[(size_t) i].filterEnv.reset();
    }
    if (currentEngine)
        currentEngine->reset();
}

void VoiceManager::renderVoice (Voice& voice, float* left, float* right, int numSamples,
                                ModulationMatrix& mod, const VoiceRenderParams& baseParams)
{
    const int voiceIndex = (int) (&voice - voices.data());
    ModulatedParams mp = {};
    static_cast<VoiceRenderParams&> (mp) = baseParams;

    voice.ampEnv.setAttack (baseParams.attack);
    voice.ampEnv.setDecay (baseParams.decay);
    voice.ampEnv.setSustain (baseParams.sustain);
    voice.ampEnv.setRelease (baseParams.release);
    voice.filterEnv.setAttack (baseParams.attack * 0.8f);
    voice.filterEnv.setDecay (baseParams.decay);
    voice.filterEnv.setSustain (baseParams.sustain);
    voice.filterEnv.setRelease (baseParams.release);

    for (int i = 0; i < numSamples; ++i)
    {
        ModulationMatrix::VoiceMod vm;
        mod.tickVoice (voiceIndex, vm);

        mp.pitchSemitones = vm.pitch + voice.mpePitch + (voice.mpePressure * 0.1f);
        mp.ampMod = vm.amp * voice.velocity;
        mp.cutoff = baseParams.cutoff * std::pow (2.0f, vm.cutoff) + vm.cutoff * 4000.0f;
        mp.cutoff += voice.filterEnv.next() * baseParams.filterEnvAmount * 6000.0f;
        mp.resonance = juce::jlimit (0.0f, 0.95f, baseParams.resonance + vm.resonance);
        mp.pulseWidth = juce::jlimit (0.05f, 0.95f, baseParams.pulseWidth + vm.pulseWidth * 0.4f);
        mp.wtPosition = juce::jlimit (0.0f, 1.0f, baseParams.wtPosition + vm.wtPosition);
        mp.fmIndex = juce::jmax (0.0f, baseParams.fmIndex + vm.fmIndex * 4.0f);
        mp.grainPosition = juce::jlimit (0.0f, 1.0f, baseParams.grainPosition + vm.grainPos);
        mp.grainDensity = juce::jlimit (0.05f, 1.0f, baseParams.grainDensity + vm.grainDensity);

        float l = 0.0f, r = 0.0f;
        if (currentEngine)
            currentEngine->render (voiceIndex, &l, &r, 1, mp);

        const float cut = juce::jlimit (40.0f, 18000.0f, mp.cutoff);
        // Parallel blend: TPT SVF clarity + Moog ladder body (Serum-ish thickness)
        voice.filter.setCutoff (cut);
        voice.filter.setResonance (mp.resonance * 0.85f);
        voice.ladder.setCutoff (cut);
        voice.ladder.setResonance (mp.resonance);
        const float lA = voice.filter.process (l);
        const float rA = voice.filter.process (r);
        const float lB = voice.ladder.process (l);
        const float rB = voice.ladder.process (r);
        l = 0.45f * lA + 0.55f * lB;
        r = 0.45f * rA + 0.55f * rB;

        const float env = voice.ampEnv.next() * mp.ampMod;
        l = std::tanh (l * env * 1.1f);
        r = std::tanh (r * env * 1.1f);

        left[i] += l;
        right[i] += r;
    }

    if (! voice.ampEnv.isActive())
        voice.active = false;
}

void VoiceManager::process (juce::AudioBuffer<float>& buffer, int numSamples,
                            ModulationMatrix& mod, const VoiceRenderParams& baseParams)
{
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;

    for (int i = 0; i < kMaxVoices; ++i)
    {
        if (voices[(size_t) i].active)
            renderVoice (voices[(size_t) i], left, right, numSamples, mod, baseParams);
    }
}
