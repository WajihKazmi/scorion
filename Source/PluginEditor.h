#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ui/ScorionLookAndFeel.h"
#include "ui/ScopeDisplay.h"
#include "ui/WavetablePad.h"
#include "ui/PianoBar.h"
#include "ui/LevelMeter.h"
#include "ui/LfoPanel.h"
#include "ui/AdsrEditor.h"
#include "ui/PortalStage.h"
#include "ui/SoundLibrary.h"
#include "ui/SectionPanel.h"
#include "ui/Spectrogram.h"
#include "ui/VoiceMeter.h"
#include "ui/InspectorSection.h"
#include "ui/MacroCard.h"
#include "ui/SettingsPanel.h"
#include "audio/PerformanceMode.h"
#include <memory>

class ScorionAudioProcessor;

class ScorionAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit ScorionAudioProcessorEditor (ScorionAudioProcessor&);
    ~ScorionAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void refreshPresetLabel();
    void applyTheme();
    void applyAtmosphereFromPreset();
    void setupKnob (juce::Slider& s, juce::Label& lab, const juce::String& text, bool hero = false, bool mod = false);
    void setCharacterMode (int mode);
    void loadSampleFile (const juce::File& file);
    void loadWavetableFile (const juce::File& file);
    void applyInitPatch();
    void auditionPreview (int midiNote = 60);
    void handleLibrarySelect (int index);
    void layoutInspector();
    void setMainTab (int tab); // 0 play, 1 settings
    void applyNamedTheme (ScorionLookAndFeel::NamedTheme t);
    void persistUiSettings();
    void restoreUiSettings();
    void applyPerformanceMode (PerformanceMode mode);

    ScorionAudioProcessor& proc;
    ScorionLookAndFeel lookAndFeel;

    juce::Label brandLabel;
    juce::Label presetLabel;
    juce::TextEditor searchBox;
    juce::TextButton initButton { "Init" };
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::TextButton panicButton { "Panic" };
    juce::TextButton warmButton { "Warm" };
    juce::TextButton balancedButton { "Bal" };
    juce::TextButton openButton { "Open" };
    juce::TextButton playTab { "Play" };
    juce::TextButton settingsTab { "Settings" };
    juce::ComboBox engineBox;

    // Global FX in header
    juce::Slider headerReverb, headerDelay;
    juce::Label headerRevL, headerDlyL;

    SectionPanel browserSection, liveSection, designSection;
    std::unique_ptr<SoundLibrary> soundLibrary;
    SettingsPanel settingsPanel;
    PortalStage portalStage;
    ScopeDisplay scope;
    Spectrogram spectrogram;
    WavetablePad wavetablePad;
    LevelMeter meter;
    VoiceMeter voiceMeter;
    LfoPanel lfo1Panel;
    AdsrEditor adsrEditor;
    std::unique_ptr<PianoBar> pianoBar;

    InspectorSection filterInspector, ampInspector, fxInspector, masterInspector;
    MacroCard macroCard1, macroCard2, macroCard3, macroCard4;

    juce::Slider cutoffSlider, resonanceSlider, attackSlider, releaseSlider;
    juce::Slider decaySlider, sustainSlider, filterEnvSlider, unisonSlider;
    juce::Slider masterSlider, reverbSlider, delaySlider;
    juce::Slider wtPosSlider, fmIndexSlider, grainDensitySlider;
    juce::Label cutoffLabel, resonanceLabel, attackLabel, releaseLabel;
    juce::Label decayLabel, sustainLabel, filterEnvLabel, unisonLabel;
    juce::Label masterLabel, reverbLabel, delayLabel;
    juce::Label wtPosLabel, fmIndexLabel, grainDensityLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> cutoffAttach, resonanceAttach, attackAttach, releaseAttach;
    std::unique_ptr<Attachment> decayAttach, sustainAttach, filterEnvAttach, unisonAttach;
    std::unique_ptr<Attachment> masterAttach, reverbAttach, delayAttach;
    std::unique_ptr<Attachment> wtPosAttach, fmIndexAttach, grainDensityAttach;
    std::unique_ptr<Attachment> macro1Attach, macro2Attach, macro3Attach, macro4Attach;
    std::unique_ptr<Attachment> headerRevAttach, headerDlyAttach;
    std::unique_ptr<Attachment> lfo1RateAttach, lfo1CutAttach, lfo1WtAttach, lfo1PanAttach, lfo1AmpAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineAttach, lfo1WaveAttach, lfo1DivAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfo1SyncAttach;

    int characterMode_ = 1;
    int mainTab_ = 0;
    bool auditionOnClick_ = true;
    bool flFriendly_ = true;
    PerformanceMode perfMode_ = PerformanceMode::Eco;
    float energySmooth_ = 0.0f;
    float backdropPhase_ = 0.0f;
    int idleFrames_ = 0;
    juce::Rectangle<int> headerBounds, contentBounds, centerBounds, designBounds, pianoBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScorionAudioProcessorEditor)
};
