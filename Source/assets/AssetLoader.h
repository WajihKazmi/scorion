#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <map>
#include <memory>
#include <vector>

struct AudioAsset
{
    std::vector<float> samples;
    double sampleRate = 44100.0;
    int rootNote = 60;
    juce::String id;
};

class AssetLoader
{
public:
    AssetLoader();
    ~AssetLoader();

    void start();
    void shutdown();
    void worker();

    juce::String requestLoad (const juce::File& file, int rootNote = 60);
    const AudioAsset* getIfReady (const juce::String& id) const noexcept;

    const AudioAsset& builtinSample() const noexcept { return builtinSample_; }
    const AudioAsset& builtinWavetable() const noexcept { return builtinWavetable_; }

private:
    void buildBuiltins();

    struct Job
    {
        juce::String id;
        juce::File file;
        int rootNote = 60;
    };

    mutable juce::CriticalSection lock_;
    std::map<juce::String, std::unique_ptr<AudioAsset>> ready_;
    std::vector<Job> queue_;
    std::atomic<bool> running_ { false };
    std::unique_ptr<juce::Thread> thread_;
    AudioAsset builtinSample_;
    AudioAsset builtinWavetable_;
};
