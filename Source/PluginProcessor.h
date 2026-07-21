#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "audio/AudioEngine.h"
#include "presets/PresetManager.h"

class ScorionAudioProcessor final : public juce::AudioProcessor
{
public:
    ScorionAudioProcessor();
    ~ScorionAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    AudioEngine& getEngine() noexcept { return engine; }
    PresetManager& getPresetManager() noexcept { return presetManager; }
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    /** Extra UI settings persisted with plugin state (FL Studio friendly). */
    juce::ValueTree& getUiSettings() noexcept { return uiSettings; }

    void panic();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioProcessorValueTreeState apvts;
    AudioEngine engine;
    PresetManager presetManager;
    juce::MidiKeyboardState keyboardState;
    juce::ValueTree uiSettings { "UISettings" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScorionAudioProcessor)
};
