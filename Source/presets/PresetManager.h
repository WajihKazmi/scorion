#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

enum class LibraryItemKind { Preset, Wavetable, Sample };

struct PresetInfo
{
    juce::String name;
    juce::String category;
    juce::StringArray tags;
    juce::String author;
    juce::String engine;
    juce::String mood;
    float energy = 0.5f;
    float brightness = 0.5f;
    float warmth = 0.5f;
    int artworkSeed = 0;
    juce::File file;
    LibraryItemKind kind = LibraryItemKind::Preset;
};

class PresetManager
{
public:
    void initialise (const juce::File& factoryDir, const juce::File& userDir);
    void rescan();

    const std::vector<PresetInfo>& presets() const noexcept { return presets_; }
    int getCurrentIndex() const noexcept { return currentIndex_; }
    juce::String getCurrentName() const;
    juce::String getCurrentCategory() const;

    bool loadPreset (int index, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& extraState);
    bool loadPresetFile (const juce::File& file, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& extraState);
    bool savePreset (const juce::String& name, const juce::String& category,
                     juce::AudioProcessorValueTreeState& apvts, const juce::ValueTree& extraState);

    void next();
    void previous();

    std::vector<int> filteredIndices (const juce::String& category, const juce::String& query,
                                      bool favoritesOnly = false) const;

    bool isFavorite (const juce::String& name) const;
    void toggleFavorite (const juce::String& name);
    void loadFavorites();
    void saveFavorites() const;

    /** Map natural-language-ish phrases to searchable tokens. */
    static juce::StringArray expandQueryTokens (const juce::String& query);

    static juce::var paramsToJson (juce::AudioProcessorValueTreeState& apvts);
    static void applyJsonParams (juce::AudioProcessorValueTreeState& apvts, const juce::var& json);

    static juce::String serializePresetJson (const juce::DynamicObject::Ptr& root);
    static juce::var parsePresetJson (const juce::String& text);

    /** Spec collection names mapped from existing categories. */
    static juce::String collectionForCategory (const juce::String& category);

private:
    void scanAssetFiles();

    juce::File factoryDir_, userDir_;
    std::vector<PresetInfo> presets_;
    juce::StringArray favorites_;
    int currentIndex_ = 0;
};
