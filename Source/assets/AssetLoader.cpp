#include "assets/AssetLoader.h"
#include <cmath>

namespace
{
class AssetWorkerThread final : public juce::Thread
{
public:
    explicit AssetWorkerThread (AssetLoader& owner)
        : juce::Thread ("ScorionAssetLoader"), owner_ (owner) {}

    void run() override { owner_.worker(); }

private:
    AssetLoader& owner_;
};
}

void AssetLoader::worker()
{
    while (running_.load())
    {
        Job job;
        bool hasJob = false;
        {
            const juce::ScopedLock sl (lock_);
            if (! queue_.empty())
            {
                job = queue_.front();
                queue_.erase (queue_.begin());
                hasJob = true;
            }
        }

        if (! hasJob)
        {
            juce::Thread::sleep (20);
            continue;
        }

        if (job.id.isNotEmpty() && job.file.existsAsFile())
        {
            juce::AudioFormatManager fm;
            fm.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (job.file));
            auto asset = std::make_unique<AudioAsset>();
            asset->id = job.id;
            asset->rootNote = job.rootNote;
            if (reader != nullptr)
            {
                asset->sampleRate = reader->sampleRate;
                juce::AudioBuffer<float> buf ((int) reader->numChannels, (int) reader->lengthInSamples);
                reader->read (&buf, 0, (int) reader->lengthInSamples, 0, true, true);
                asset->samples.resize ((size_t) buf.getNumSamples());
                for (int i = 0; i < buf.getNumSamples(); ++i)
                {
                    float s = buf.getSample (0, i);
                    if (buf.getNumChannels() > 1)
                        s = 0.5f * (s + buf.getSample (1, i));
                    asset->samples[(size_t) i] = s;
                }
            }
            const juce::ScopedLock sl (lock_);
            ready_[job.id] = std::move (asset);
        }
    }
}

AssetLoader::AssetLoader()
{
    buildBuiltins();
}

AssetLoader::~AssetLoader()
{
    shutdown();
}

void AssetLoader::buildBuiltins()
{
    builtinSample_.id = "builtin://sample/pluck";
    builtinSample_.sampleRate = 44100.0;
    builtinSample_.rootNote = 57;
    builtinSample_.samples.resize (44100);
    for (size_t i = 0; i < builtinSample_.samples.size(); ++i)
    {
        const float t = (float) i / 44100.0f;
        const float env = std::exp (-3.5f * t);
        float s = 0.0f;
        for (int h = 1; h <= 5; ++h)
            s += std::sin (2.0f * 3.14159265f * 220.0f * (float) h * t) / (float) h;
        builtinSample_.samples[i] = s * env * 0.45f;
    }

    builtinWavetable_.id = "builtin://wavetable/harmonic";
    builtinWavetable_.sampleRate = 44100.0;
    constexpr int frameSize = 2048;
    constexpr int frames = 4;
    builtinWavetable_.samples.resize ((size_t) (frameSize * frames));
    for (int f = 0; f < frames; ++f)
    {
        const int harmonics = 2 + f * 6;
        for (int i = 0; i < frameSize; ++i)
        {
            float s = 0.0f;
            const float phase = (float) i / (float) frameSize;
            for (int h = 1; h <= harmonics; ++h)
                s += std::sin (phase * 6.28318530718f * (float) h) / (float) h;
            builtinWavetable_.samples[(size_t) (f * frameSize + i)] = s * 0.4f;
        }
    }
}

void AssetLoader::start()
{
    if (running_.exchange (true))
        return;
    thread_ = std::make_unique<AssetWorkerThread> (*this);
    thread_->startThread();
}

void AssetLoader::shutdown()
{
    running_.store (false);
    if (thread_ != nullptr)
    {
        thread_->stopThread (1000);
        thread_.reset();
    }
}

juce::String AssetLoader::requestLoad (const juce::File& file, int rootNote)
{
    const auto id = "file://" + file.getFullPathName();
    const juce::ScopedLock sl (lock_);
    queue_.push_back ({ id, file, rootNote });
    return id;
}

const AudioAsset* AssetLoader::getIfReady (const juce::String& id) const noexcept
{
    const juce::ScopedLock sl (lock_);
    auto it = ready_.find (id);
    if (it == ready_.end())
        return nullptr;
    return it->second.get();
}
