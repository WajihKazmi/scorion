#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ui/Motion.h"
#include <cmath>

void ScorionAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& lab, const juce::String& text, bool hero, bool mod)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setPopupDisplayEnabled (true, true, this);
    s.setDoubleClickReturnValue (true, 0.5);
    s.setVelocityBasedMode (true);
    s.setVelocityModeParameters (1.0, 1, 0.08, true); // shift = fine
    if (hero) s.setName ("hero");
    if (mod) { s.setName ("mod"); s.setComponentID ("mod"); }
    addAndMakeVisible (s);
    lab.setText (text, juce::dontSendNotification);
    lab.setJustificationType (juce::Justification::centred);
    lab.setColour (juce::Label::textColourId, lookAndFeel.textSecondary());
    lab.setFont (lookAndFeel.labelFont());
    addAndMakeVisible (lab);
}

void ScorionAudioProcessorEditor::setCharacterMode (int mode)
{
    characterMode_ = juce::jlimit (0, 2, mode);
    warmButton.setToggleState (characterMode_ == 0, juce::dontSendNotification);
    balancedButton.setToggleState (characterMode_ == 1, juce::dontSendNotification);
    openButton.setToggleState (characterMode_ == 2, juce::dontSendNotification);

    auto& apvts = proc.getAPVTS();
    auto set = [&] (const char* id, float v) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (v));
    };

    if (characterMode_ == 0)
    {
        set ("filterCutoff", 2800.0f);
        set ("reverbMix", 0.42f);
        set ("filterResonance", 0.22f);
    }
    else if (characterMode_ == 1)
    {
        set ("filterCutoff", 5500.0f);
        set ("reverbMix", 0.28f);
        set ("filterResonance", 0.25f);
    }
    else
    {
        set ("filterCutoff", 11000.0f);
        set ("reverbMix", 0.35f);
        set ("filterResonance", 0.18f);
    }
}

void ScorionAudioProcessorEditor::applyInitPatch()
{
    auto& apvts = proc.getAPVTS();
    auto set = [&] (const char* id, float v) {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (v));
    };
    set ("engine", 0.0f);
    set ("masterGain", 0.75f);
    set ("filterCutoff", 6000.0f);
    set ("filterResonance", 0.2f);
    set ("ampAttack", 0.01f);
    set ("ampDecay", 0.25f);
    set ("ampSustain", 0.7f);
    set ("ampRelease", 0.35f);
    set ("reverbMix", 0.12f);
    set ("delayMix", 0.05f);
    set ("lfo1ToCutoff", 0.0f);
    set ("lfo1Rate", 2.0f);
    set ("unisonCount", 0.0f);
    presetLabel.setText ("Init", juce::dontSendNotification);
    adsrEditor.setValues (0.01f, 0.25f, 0.7f, 0.35f);
}

void ScorionAudioProcessorEditor::loadSampleFile (const juce::File& file)
{
    if (auto* p = proc.getAPVTS().getParameter ("engine"))
        p->setValueNotifyingHost (p->convertTo0to1 (4.0f));
    juce::Timer::callAfterDelay (40, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this), file] {
        if (safe != nullptr)
            safe->proc.getEngine().voices().loadSampleFromFile (file, 60);
    });
}

void ScorionAudioProcessorEditor::loadWavetableFile (const juce::File& file)
{
    if (auto* p = proc.getAPVTS().getParameter ("engine"))
        p->setValueNotifyingHost (p->convertTo0to1 (1.0f));
    juce::Timer::callAfterDelay (40, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this), file] {
        if (safe != nullptr)
            safe->proc.getEngine().voices().loadWavetableFromFile (file);
    });
}

void ScorionAudioProcessorEditor::auditionPreview (int midiNote)
{
    if (! auditionOnClick_) return;
    proc.getKeyboardState().noteOn (1, midiNote, 0.85f);
    juce::Timer::callAfterDelay (320, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this), midiNote] {
        if (safe != nullptr)
            safe->proc.getKeyboardState().noteOff (1, midiNote, 0.0f);
    });
}

void ScorionAudioProcessorEditor::handleLibrarySelect (int index)
{
    const auto& presets = proc.getPresetManager().presets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size())) return;
    const auto& info = presets[(size_t) index];

    if (info.kind == LibraryItemKind::Sample)
    {
        loadSampleFile (info.file);
        juce::Timer::callAfterDelay (80, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this)] {
            if (safe != nullptr) safe->auditionPreview (60);
        });
        presetLabel.setText (info.name, juce::dontSendNotification);
        applyAtmosphereFromPreset();
        return;
    }
    if (info.kind == LibraryItemKind::Wavetable)
    {
        loadWavetableFile (info.file);
        juce::Timer::callAfterDelay (80, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this)] {
            if (safe != nullptr) safe->auditionPreview (60);
        });
        presetLabel.setText (info.name, juce::dontSendNotification);
        applyAtmosphereFromPreset();
        return;
    }

    juce::ValueTree extra ("extra");
    proc.getPresetManager().loadPreset (index, proc.getAPVTS(), extra);
    refreshPresetLabel();
    auditionPreview (60);
    if (soundLibrary) soundLibrary->setSelectedIndex (index);
}

void ScorionAudioProcessorEditor::applyAtmosphereFromPreset()
{
    const auto cat = proc.getPresetManager().getCurrentCategory().toLowerCase();
    juce::Colour tint = lookAndFeel.mint(); // cyan default
    if (cat.contains ("bass") || cat.contains ("moog") || cat.contains ("dark"))
        tint = lookAndFeel.ember(); // white neon
    else if (cat.contains ("pad") || cat.contains ("choir") || cat.contains ("atmos"))
        tint = lookAndFeel.mint().withBrightness (0.85f);
    else if (cat.contains ("lead") || cat.contains ("pop"))
        tint = juce::Colours::white;
    lookAndFeel.setAtmosphereTint (tint);
    repaint();
}

ScorionAudioProcessorEditor::ScorionAudioProcessorEditor (ScorionAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lookAndFeel);
    applyTheme();

    brandLabel.setText ("SCORION", juce::dontSendNotification);
    brandLabel.setFont (lookAndFeel.brandFont (26.0f));
    brandLabel.setColour (juce::Label::textColourId, lookAndFeel.textPrimary());
    addAndMakeVisible (brandLabel);

    presetLabel.setFont (lookAndFeel.uiFont (13.0f, true));
    presetLabel.setColour (juce::Label::textColourId, lookAndFeel.textPrimary());
    addAndMakeVisible (presetLabel);

    searchBox.setTextToShowWhenEmpty ("Universal search...", lookAndFeel.textSecondary());
    searchBox.onTextChange = [this] {
        if (soundLibrary) soundLibrary->setExternalQuery (searchBox.getText());
    };
    addAndMakeVisible (searchBox);

    for (auto* b : { &initButton, &prevButton, &nextButton, &panicButton,
                     &warmButton, &balancedButton, &openButton, &playTab, &settingsTab })
        addAndMakeVisible (*b);

    playTab.setClickingTogglesState (true);
    settingsTab.setClickingTogglesState (true);
    playTab.setRadioGroupId (2201);
    settingsTab.setRadioGroupId (2201);
    playTab.setToggleState (true, juce::dontSendNotification);
    playTab.onClick = [this] { setMainTab (0); };
    settingsTab.onClick = [this] { setMainTab (1); };

    initButton.onClick = [this] { applyInitPatch(); };
    warmButton.setClickingTogglesState (true);
    balancedButton.setClickingTogglesState (true);
    openButton.setClickingTogglesState (true);
    warmButton.setRadioGroupId (1001);
    balancedButton.setRadioGroupId (1001);
    openButton.setRadioGroupId (1001);
    warmButton.onClick = [this] { setCharacterMode (0); };
    balancedButton.onClick = [this] { setCharacterMode (1); };
    openButton.onClick = [this] { setCharacterMode (2); };
    balancedButton.setToggleState (true, juce::dontSendNotification);

    prevButton.onClick = [this] {
        proc.getPresetManager().previous();
        juce::ValueTree extra ("extra");
        proc.getPresetManager().loadPreset (proc.getPresetManager().getCurrentIndex(), proc.getAPVTS(), extra);
        refreshPresetLabel();
        if (soundLibrary) soundLibrary->setSelectedIndex (proc.getPresetManager().getCurrentIndex());
    };
    nextButton.onClick = [this] {
        proc.getPresetManager().next();
        juce::ValueTree extra ("extra");
        proc.getPresetManager().loadPreset (proc.getPresetManager().getCurrentIndex(), proc.getAPVTS(), extra);
        refreshPresetLabel();
        if (soundLibrary) soundLibrary->setSelectedIndex (proc.getPresetManager().getCurrentIndex());
    };
    panicButton.onClick = [this] { proc.panic(); };

    engineBox.addItemList ({ "Virtual Analog", "Wavetable", "FM", "Granular", "Sampler" }, 1);
    addAndMakeVisible (engineBox);
    engineAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.getAPVTS(), "engine", engineBox);

    setupKnob (headerReverb, headerRevL, "FX Rev", false, false);
    setupKnob (headerDelay, headerDlyL, "FX Dly", false, false);
    headerRevAttach = std::make_unique<Attachment> (proc.getAPVTS(), "reverbMix", headerReverb);
    headerDlyAttach = std::make_unique<Attachment> (proc.getAPVTS(), "delayMix", headerDelay);

    browserSection.setLookAndFeelRef (&lookAndFeel);
    browserSection.setTitle ("BROWSER");
    browserSection.setSubtitle ("LIBRARY");
    addAndMakeVisible (browserSection);

    soundLibrary = std::make_unique<SoundLibrary> (proc.getPresetManager());
    soundLibrary->setLookAndFeelRef (&lookAndFeel);
    soundLibrary->onSelect = [this] (int i) { handleLibrarySelect (i); };
    soundLibrary->onFavorite = [this] (int i) {
        const auto& presets = proc.getPresetManager().presets();
        if (juce::isPositiveAndBelow (i, (int) presets.size()))
        {
            proc.getPresetManager().toggleFavorite (presets[(size_t) i].name);
            soundLibrary->refreshFromManager();
        }
    };
    soundLibrary->onPreview = [this] (int) { /* audition already in onSelect */ };
    addAndMakeVisible (*soundLibrary);

    liveSection.setLookAndFeelRef (&lookAndFeel);
    liveSection.setTitle ("LIVE STAGE");
    liveSection.setSubtitle ("PORTAL");
    addAndMakeVisible (liveSection);

    designSection.setLookAndFeelRef (&lookAndFeel);
    designSection.setTitle ("DESIGN");
    designSection.setSubtitle ("INSPECTOR");
    addAndMakeVisible (designSection);

    portalStage.setLookAndFeelRef (&lookAndFeel);
    scope.setLookAndFeelRef (&lookAndFeel);
    spectrogram.setLookAndFeelRef (&lookAndFeel);
    wavetablePad.setLookAndFeelRef (&lookAndFeel);
    meter.setLookAndFeelRef (&lookAndFeel);
    voiceMeter.setLookAndFeelRef (&lookAndFeel);
    addAndMakeVisible (portalStage);
    addAndMakeVisible (scope);
    addAndMakeVisible (spectrogram);
    addAndMakeVisible (wavetablePad);
    addAndMakeVisible (meter);
    addAndMakeVisible (voiceMeter);

    lfo1Panel.setLookAndFeelRef (&lookAndFeel);
    lfo1Panel.setTitle ("LFO 1  -  MOTION");
    addAndMakeVisible (lfo1Panel);
    lfo1RateAttach = std::make_unique<Attachment> (proc.getAPVTS(), "lfo1Rate", lfo1Panel.rateSlider);
    lfo1CutAttach = std::make_unique<Attachment> (proc.getAPVTS(), "lfo1ToCutoff", lfo1Panel.cutSlider);
    lfo1WtAttach = std::make_unique<Attachment> (proc.getAPVTS(), "lfo1ToWt", lfo1Panel.wtSlider);
    lfo1PanAttach = std::make_unique<Attachment> (proc.getAPVTS(), "lfo1ToPan", lfo1Panel.panSlider);
    lfo1AmpAttach = std::make_unique<Attachment> (proc.getAPVTS(), "lfo1ToAmp", lfo1Panel.ampSlider);
    lfo1WaveAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.getAPVTS(), "lfo1Wave", lfo1Panel.waveBox);
    lfo1DivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.getAPVTS(), "lfo1Division", lfo1Panel.divBox);
    lfo1SyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.getAPVTS(), "lfo1Sync", lfo1Panel.syncButton);

    for (auto* m : { &macroCard1, &macroCard2, &macroCard3, &macroCard4 })
    {
        m->setLookAndFeelRef (&lookAndFeel);
        addAndMakeVisible (*m);
    }
    macroCard1.setTitle ("M1");
    macroCard2.setTitle ("M2");
    macroCard3.setTitle ("M3");
    macroCard4.setTitle ("M4");
    macro1Attach = std::make_unique<Attachment> (proc.getAPVTS(), "macro1", macroCard1.getKnob());
    macro2Attach = std::make_unique<Attachment> (proc.getAPVTS(), "macro2", macroCard2.getKnob());
    macro3Attach = std::make_unique<Attachment> (proc.getAPVTS(), "macro3", macroCard3.getKnob());
    macro4Attach = std::make_unique<Attachment> (proc.getAPVTS(), "macro4", macroCard4.getKnob());

    setupKnob (cutoffSlider, cutoffLabel, "Cutoff");
    setupKnob (resonanceSlider, resonanceLabel, "Reso");
    setupKnob (attackSlider, attackLabel, "Atk");
    setupKnob (decaySlider, decayLabel, "Dec");
    setupKnob (sustainSlider, sustainLabel, "Sus");
    setupKnob (releaseSlider, releaseLabel, "Rel");
    setupKnob (filterEnvSlider, filterEnvLabel, "FiltEnv");
    setupKnob (unisonSlider, unisonLabel, "Detune");
    setupKnob (reverbSlider, reverbLabel, "Rev");
    setupKnob (delaySlider, delayLabel, "Dly");
    setupKnob (wtPosSlider, wtPosLabel, "Table");
    setupKnob (fmIndexSlider, fmIndexLabel, "FM");
    setupKnob (grainDensitySlider, grainDensityLabel, "Grain");
    setupKnob (masterSlider, masterLabel, "Amount", true);

    cutoffAttach = std::make_unique<Attachment> (proc.getAPVTS(), "filterCutoff", cutoffSlider);
    resonanceAttach = std::make_unique<Attachment> (proc.getAPVTS(), "filterResonance", resonanceSlider);
    attackAttach = std::make_unique<Attachment> (proc.getAPVTS(), "ampAttack", attackSlider);
    decayAttach = std::make_unique<Attachment> (proc.getAPVTS(), "ampDecay", decaySlider);
    sustainAttach = std::make_unique<Attachment> (proc.getAPVTS(), "ampSustain", sustainSlider);
    releaseAttach = std::make_unique<Attachment> (proc.getAPVTS(), "ampRelease", releaseSlider);
    filterEnvAttach = std::make_unique<Attachment> (proc.getAPVTS(), "filterEnvAmount", filterEnvSlider);
    unisonAttach = std::make_unique<Attachment> (proc.getAPVTS(), "unisonDetune", unisonSlider);
    reverbAttach = std::make_unique<Attachment> (proc.getAPVTS(), "reverbMix", reverbSlider);
    delayAttach = std::make_unique<Attachment> (proc.getAPVTS(), "delayMix", delaySlider);
    wtPosAttach = std::make_unique<Attachment> (proc.getAPVTS(), "wtPosition", wtPosSlider);
    fmIndexAttach = std::make_unique<Attachment> (proc.getAPVTS(), "fmIndex", fmIndexSlider);
    grainDensityAttach = std::make_unique<Attachment> (proc.getAPVTS(), "grainDensity", grainDensitySlider);
    masterAttach = std::make_unique<Attachment> (proc.getAPVTS(), "masterGain", masterSlider);

    adsrEditor.setLookAndFeelRef (&lookAndFeel);
    adsrEditor.onChanged = [this] (float a, float d, float s, float r) {
        auto set = [&] (const char* id, float v) {
            if (auto* p = proc.getAPVTS().getParameter (id))
                p->setValueNotifyingHost (p->convertTo0to1 (v));
        };
        set ("ampAttack", a);
        set ("ampDecay", d);
        set ("ampSustain", s);
        set ("ampRelease", r);
    };
    addAndMakeVisible (adsrEditor);

    auto wireInspector = [this] (InspectorSection& sec, const juce::String& title) {
        sec.setLookAndFeelRef (&lookAndFeel);
        sec.setTitle (title);
        sec.onOpenChanged = [this] { resized(); };
        addAndMakeVisible (sec);
    };
    wireInspector (filterInspector, "FILTER");
    wireInspector (ampInspector, "AMP / ENVELOPE");
    wireInspector (fxInspector, "SPACE / ENGINE");
    wireInspector (masterInspector, "MASTER");

    // Reparent knobs into inspector bodies for clean collapse
    auto adopt = [] (InspectorSection& sec, juce::Component& c) {
        sec.getBody().addAndMakeVisible (c);
    };
    adopt (filterInspector, cutoffSlider); adopt (filterInspector, cutoffLabel);
    adopt (filterInspector, resonanceSlider); adopt (filterInspector, resonanceLabel);
    adopt (filterInspector, filterEnvSlider); adopt (filterInspector, filterEnvLabel);
    adopt (filterInspector, unisonSlider); adopt (filterInspector, unisonLabel);

    adopt (ampInspector, adsrEditor);
    adopt (ampInspector, attackSlider); adopt (ampInspector, attackLabel);
    adopt (ampInspector, decaySlider); adopt (ampInspector, decayLabel);
    adopt (ampInspector, sustainSlider); adopt (ampInspector, sustainLabel);
    adopt (ampInspector, releaseSlider); adopt (ampInspector, releaseLabel);

    adopt (fxInspector, reverbSlider); adopt (fxInspector, reverbLabel);
    adopt (fxInspector, delaySlider); adopt (fxInspector, delayLabel);
    adopt (fxInspector, wtPosSlider); adopt (fxInspector, wtPosLabel);
    adopt (fxInspector, fmIndexSlider); adopt (fxInspector, fmIndexLabel);

    adopt (masterInspector, grainDensitySlider); adopt (masterInspector, grainDensityLabel);
    adopt (masterInspector, masterSlider); adopt (masterInspector, masterLabel);

    pianoBar = std::make_unique<PianoBar> (proc.getKeyboardState());
    pianoBar->setLookAndFeelRef (&lookAndFeel);
    addAndMakeVisible (*pianoBar);
    pianoBar->getKeyboard().setMidiChannel (1);
    pianoBar->getKeyboard().setVelocity (0.85f, true);

    settingsPanel.setLookAndFeelRef (&lookAndFeel);
    settingsPanel.onThemeChanged = [this] (ScorionLookAndFeel::NamedTheme t) { applyNamedTheme (t); };
    settingsPanel.onUiScaleChanged = [this] (float s) {
        lookAndFeel.setUiScale (s);
        setScaleFactor (s);
        persistUiSettings();
        resized();
    };
    settingsPanel.onKnobScalesChanged = [this] (bool on) {
        lookAndFeel.setShowKnobScales (on);
        persistUiSettings();
        repaint();
    };
    settingsPanel.onAuditionChanged = [this] (bool on) {
        auditionOnClick_ = on;
        persistUiSettings();
    };
    settingsPanel.onFlFriendlyChanged = [this] (bool on) {
        flFriendly_ = on;
        persistUiSettings();
        if (flFriendly_ && pianoBar != nullptr)
            pianoBar->getKeyboard().grabKeyboardFocus();
    };
    settingsPanel.onMidiChannelChanged = [this] (int ch) {
        if (pianoBar != nullptr)
            pianoBar->getKeyboard().setMidiChannel (ch <= 0 ? 1 : ch);
        persistUiSettings();
    };
    addChildComponent (settingsPanel);
    settingsPanel.setVisible (false);

    setSize (1600, 1000);
    setResizable (true, true);
    setResizeLimits (1280, 840, 2800, 1700);
    restoreUiSettings();
    refreshPresetLabel();
    startTimerHz (48);
    setMainTab (0);

    juce::Timer::callAfterDelay (100, [safe = juce::Component::SafePointer<ScorionAudioProcessorEditor> (this)] {
        if (safe != nullptr && safe->pianoBar != nullptr)
            safe->pianoBar->getKeyboard().grabKeyboardFocus();
    });
}

ScorionAudioProcessorEditor::~ScorionAudioProcessorEditor()
{
    if (pianoBar != nullptr)
        pianoBar->getKeyboard().setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void ScorionAudioProcessorEditor::applyTheme()
{
    brandLabel.setFont (lookAndFeel.brandFont (26.0f));
    brandLabel.setColour (juce::Label::textColourId, lookAndFeel.textPrimary());
    presetLabel.setFont (lookAndFeel.uiFont (14.0f, true));
    presetLabel.setColour (juce::Label::textColourId, lookAndFeel.textPrimary());
    browserSection.setLookAndFeelRef (&lookAndFeel);
    liveSection.setLookAndFeelRef (&lookAndFeel);
    designSection.setLookAndFeelRef (&lookAndFeel);
    if (soundLibrary) soundLibrary->setLookAndFeelRef (&lookAndFeel);
    settingsPanel.setLookAndFeelRef (&lookAndFeel);
    settingsPanel.syncFromLookAndFeel (lookAndFeel);
    portalStage.setLookAndFeelRef (&lookAndFeel);
    scope.setLookAndFeelRef (&lookAndFeel);
    spectrogram.setLookAndFeelRef (&lookAndFeel);
    wavetablePad.setLookAndFeelRef (&lookAndFeel);
    meter.setLookAndFeelRef (&lookAndFeel);
    voiceMeter.setLookAndFeelRef (&lookAndFeel);
    lfo1Panel.setLookAndFeelRef (&lookAndFeel);
    adsrEditor.setLookAndFeelRef (&lookAndFeel);
    for (auto* m : { &macroCard1, &macroCard2, &macroCard3, &macroCard4 })
        m->setLookAndFeelRef (&lookAndFeel);
    filterInspector.setLookAndFeelRef (&lookAndFeel);
    ampInspector.setLookAndFeelRef (&lookAndFeel);
    fxInspector.setLookAndFeelRef (&lookAndFeel);
    masterInspector.setLookAndFeelRef (&lookAndFeel);
    if (pianoBar) pianoBar->setLookAndFeelRef (&lookAndFeel);
    repaint();
}

void ScorionAudioProcessorEditor::applyNamedTheme (ScorionLookAndFeel::NamedTheme t)
{
    lookAndFeel.setNamedTheme (t);
    applyTheme();
    persistUiSettings();
}

void ScorionAudioProcessorEditor::persistUiSettings()
{
    auto& s = proc.getUiSettings();
    s.setProperty ("theme", (int) lookAndFeel.getNamedTheme(), nullptr);
    s.setProperty ("uiScale", lookAndFeel.getUiScale(), nullptr);
    s.setProperty ("knobScales", lookAndFeel.getShowKnobScales(), nullptr);
    s.setProperty ("audition", auditionOnClick_, nullptr);
    s.setProperty ("flFriendly", flFriendly_, nullptr);
    if (pianoBar != nullptr)
        s.setProperty ("midiChannel", pianoBar->getKeyboard().getMidiChannel(), nullptr);
}

void ScorionAudioProcessorEditor::restoreUiSettings()
{
    const auto& s = proc.getUiSettings();
    if (s.hasProperty ("theme"))
        lookAndFeel.setNamedTheme ((ScorionLookAndFeel::NamedTheme) (int) s.getProperty ("theme"));
    if (s.hasProperty ("uiScale"))
    {
        lookAndFeel.setUiScale ((float) s.getProperty ("uiScale"));
        setScaleFactor (lookAndFeel.getUiScale());
    }
    if (s.hasProperty ("knobScales"))
        lookAndFeel.setShowKnobScales ((bool) s.getProperty ("knobScales"));
    if (s.hasProperty ("audition"))
        auditionOnClick_ = (bool) s.getProperty ("audition");
    if (s.hasProperty ("flFriendly"))
        flFriendly_ = (bool) s.getProperty ("flFriendly");
    if (s.hasProperty ("midiChannel") && pianoBar != nullptr)
        pianoBar->getKeyboard().setMidiChannel ((int) s.getProperty ("midiChannel"));
    settingsPanel.setAudition (auditionOnClick_);
    settingsPanel.setFlFriendly (flFriendly_);
    applyTheme();
}

void ScorionAudioProcessorEditor::setMainTab (int tab)
{
    mainTab_ = juce::jlimit (0, 1, tab);
    playTab.setToggleState (mainTab_ == 0, juce::dontSendNotification);
    settingsTab.setToggleState (mainTab_ == 1, juce::dontSendNotification);
    const bool play = mainTab_ == 0;
    browserSection.setVisible (play);
    liveSection.setVisible (play);
    designSection.setVisible (play);
    if (soundLibrary) soundLibrary->setVisible (play);
    portalStage.setVisible (play);
    scope.setVisible (play);
    spectrogram.setVisible (play);
    wavetablePad.setVisible (play);
    meter.setVisible (play);
    voiceMeter.setVisible (play);
    lfo1Panel.setVisible (play);
    for (auto* m : { &macroCard1, &macroCard2, &macroCard3, &macroCard4 })
        m->setVisible (play);
    filterInspector.setVisible (play);
    ampInspector.setVisible (play);
    fxInspector.setVisible (play);
    masterInspector.setVisible (play);
    settingsPanel.setVisible (! play);
    resized();
}

void ScorionAudioProcessorEditor::refreshPresetLabel()
{
    presetLabel.setText (proc.getPresetManager().getCurrentName(), juce::dontSendNotification);
    if (auto* a = proc.getAPVTS().getRawParameterValue ("ampAttack"))
        if (auto* d = proc.getAPVTS().getRawParameterValue ("ampDecay"))
            if (auto* s = proc.getAPVTS().getRawParameterValue ("ampSustain"))
                if (auto* r = proc.getAPVTS().getRawParameterValue ("ampRelease"))
                    adsrEditor.setValues (a->load(), d->load(), s->load(), r->load());
    liveSection.setSubtitle (proc.getPresetManager().getCurrentName().toUpperCase());
    applyAtmosphereFromPreset();
}

void ScorionAudioProcessorEditor::timerCallback()
{
    std::array<float, 256> snap {};
    proc.getEngine().probe().copySnapshot (snap);
    float energy = 0.0f;
    for (auto s : snap) energy += s * s;
    energy = std::sqrt (energy / (float) snap.size());
    energySmooth_ += 0.22f * (juce::jlimit (0.0f, 1.0f, energy * 5.0f) - energySmooth_);
    backdropPhase_ += 0.02f + energySmooth_ * 0.04f;

    // Idle throttling: drop to ~20Hz when silent
    if (energySmooth_ < 0.02f)
    {
        ++idleFrames_;
        if (idleFrames_ > 30 && (idleFrames_ % 2) != 0)
            return;
    }
    else
        idleFrames_ = 0;

    portalStage.setEnergy (energySmooth_);
    portalStage.setSnapshot (snap);
    scope.setEnergy (energySmooth_);
    scope.setAnimPhase (energySmooth_ * 6.0f + backdropPhase_);
    scope.setSnapshot (snap);

    std::array<float, 64> bands {};
    for (int b = 0; b < 64; ++b)
    {
        const int a0 = b * 256 / 64;
        const int a1 = (b + 1) * 256 / 64;
        float e = 0.0f;
        for (int i = a0; i < a1; ++i) e += snap[(size_t) i] * snap[(size_t) i];
        bands[(size_t) b] = juce::jlimit (0.0f, 1.0f, std::sqrt (e / (float) juce::jmax (1, a1 - a0)) * 6.0f);
    }
    spectrogram.pushBands (bands);
    spectrogram.setEnergy (energySmooth_);

    meter.setLevel (energySmooth_);
    meter.setPulse (energySmooth_);
    voiceMeter.setVoiceCount (proc.getEngine().voices().getActiveVoiceCount());
    voiceMeter.setEnergy (energySmooth_);
    wavetablePad.setEnergy (energySmooth_);
    adsrEditor.setEnergy (energySmooth_);
    browserSection.setEnergy (energySmooth_);
    liveSection.setEnergy (energySmooth_);
    designSection.setEnergy (energySmooth_);
    if (soundLibrary) soundLibrary->setEnergy (energySmooth_);
    if (pianoBar) pianoBar->setEnergy (energySmooth_);
    if (auto* pv = proc.getAPVTS().getRawParameterValue ("wtPosition"))
        wavetablePad.setPosition (pv->load());
}

void ScorionAudioProcessorEditor::mouseDown (const juce::MouseEvent&)
{
    if (pianoBar != nullptr)
        pianoBar->getKeyboard().grabKeyboardFocus();
}

void ScorionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (lookAndFeel.background());

    auto b = getLocalBounds().toFloat();
    const float cycle = backdropPhase_ / (Motion::bgCycleMs / 1000.0f * 48.0f);
    const auto tint = lookAndFeel.atmosphere();

    {
        const float px = b.getWidth() * (0.22f + 0.12f * std::sin (cycle * juce::MathConstants<float>::twoPi));
        const float py = b.getHeight() * (0.32f + 0.1f * std::cos (cycle * 0.7f * juce::MathConstants<float>::twoPi));
        juce::ColourGradient a (lookAndFeel.ember().withAlpha (0.08f + energySmooth_ * 0.1f),
                                px, py, juce::Colours::transparentBlack, px + 400.0f, py + 260.0f, true);
        g.setGradientFill (a);
        g.fillEllipse (px - 200.0f, py - 160.0f, 400.0f, 320.0f);

        const float qx = b.getWidth() * (0.72f + 0.08f * std::cos (cycle * 0.5f * juce::MathConstants<float>::twoPi));
        const float qy = b.getHeight() * (0.58f + 0.08f * std::sin (cycle * 0.6f * juce::MathConstants<float>::twoPi));
        juce::ColourGradient c (tint.withAlpha (0.09f + energySmooth_ * 0.08f),
                                qx, qy, juce::Colours::transparentBlack, qx + 360.0f, qy + 280.0f, true);
        g.setGradientFill (c);
        g.fillEllipse (qx - 180.0f, qy - 140.0f, 360.0f, 280.0f);
    }

    // Header chrome
    lookAndFeel.drawPanel (g, headerBounds.toFloat(), 14.0f);
    g.setColour (lookAndFeel.border());
    g.drawRect (getLocalBounds().toFloat().reduced (4.0f), 1.0f);
}

void ScorionAudioProcessorEditor::layoutInspector()
{
    auto d = designSection.contentBounds() + designSection.getBounds().getPosition();
    const int filterH = filterInspector.preferredHeight (100);
    const int ampH = ampInspector.preferredHeight (160);
    const int fxH = fxInspector.preferredHeight (100);
    const int masterH = masterInspector.preferredHeight (100);

    filterInspector.setBounds (d.removeFromTop (filterH).reduced (2));
    d.removeFromTop (8);
    ampInspector.setBounds (d.removeFromTop (ampH).reduced (2));
    d.removeFromTop (8);
    fxInspector.setBounds (d.removeFromTop (fxH).reduced (2));
    d.removeFromTop (8);
    masterInspector.setBounds (d.removeFromTop (masterH).reduced (2));

    auto place4 = [] (juce::Component& body, juce::Slider& a, juce::Label& al,
                      juce::Slider& b, juce::Label& bl,
                      juce::Slider& c, juce::Label& cl,
                      juce::Slider& dkn, juce::Label& dl, int y0 = 0) {
        auto row = body.getLocalBounds().withTrimmedTop (y0).removeFromTop (92);
        const int w = row.getWidth() / 4;
        auto cell = [&] (juce::Slider& s, juce::Label& l) {
            auto c = row.removeFromLeft (w).reduced (4);
            l.setBounds (c.removeFromBottom (14));
            s.setBounds (c);
        };
        cell (a, al); cell (b, bl); cell (c, cl); cell (dkn, dl);
    };

    place4 (filterInspector.getBody(), cutoffSlider, cutoffLabel, resonanceSlider, resonanceLabel,
            filterEnvSlider, filterEnvLabel, unisonSlider, unisonLabel);

    {
        auto body = ampInspector.getBody().getLocalBounds();
        adsrEditor.setBounds (body.removeFromTop (88).reduced (2));
        body.removeFromTop (4);
        auto row = body.removeFromTop (84);
        const int w = row.getWidth() / 4;
        auto cell = [&] (juce::Slider& s, juce::Label& l) {
            auto c = row.removeFromLeft (w).reduced (4);
            l.setBounds (c.removeFromBottom (14));
            s.setBounds (c);
        };
        cell (attackSlider, attackLabel);
        cell (decaySlider, decayLabel);
        cell (sustainSlider, sustainLabel);
        cell (releaseSlider, releaseLabel);
    }

    place4 (fxInspector.getBody(), reverbSlider, reverbLabel, delaySlider, delayLabel,
            wtPosSlider, wtPosLabel, fmIndexSlider, fmIndexLabel);

    {
        auto row = masterInspector.getBody().getLocalBounds().removeFromTop (96);
        const int w = row.getWidth() / 2;
        auto cell = [&] (juce::Slider& s, juce::Label& l) {
            auto c = row.removeFromLeft (w).reduced (6);
            l.setBounds (c.removeFromBottom (14));
            s.setBounds (c);
        };
        cell (grainDensitySlider, grainDensityLabel);
        cell (masterSlider, masterLabel);
    }
}

void ScorionAudioProcessorEditor::resized()
{
    const int gutter = 16;
    const int gap = 8;
    auto area = getLocalBounds().reduced (gutter);

    auto contentOf = [] (SectionPanel& s) -> juce::Rectangle<int> {
        return s.contentBounds() + s.getBounds().getPosition();
    };

    headerBounds = area.removeFromTop (72);
    {
        auto h = headerBounds.reduced (12, 10);
        brandLabel.setBounds (h.removeFromLeft (132));
        auto tabs = h.removeFromLeft (168);
        playTab.setBounds (tabs.removeFromLeft (80).reduced (2));
        settingsTab.setBounds (tabs.reduced (2));
        searchBox.setBounds (h.removeFromLeft (180).reduced (4, 4));
        initButton.setBounds (h.removeFromLeft (52).reduced (2));
        prevButton.setBounds (h.removeFromLeft (32).reduced (2));
        nextButton.setBounds (h.removeFromLeft (32).reduced (2));
        presetLabel.setBounds (h.removeFromLeft (130));
        engineBox.setBounds (h.removeFromLeft (130).reduced (4));
        auto modes = h.removeFromLeft (168);
        const int w = modes.getWidth() / 3;
        warmButton.setBounds (modes.removeFromLeft (w).reduced (2));
        balancedButton.setBounds (modes.removeFromLeft (w).reduced (2));
        openButton.setBounds (modes.reduced (2));

        auto fx = h.removeFromLeft (128);
        auto fxTop = fx.removeFromTop (12);
        headerRevL.setBounds (fxTop.removeFromLeft (56));
        headerDlyL.setBounds (fxTop.removeFromLeft (56));
        headerReverb.setBounds (fx.removeFromLeft (56).reduced (2));
        headerDelay.setBounds (fx.removeFromLeft (56).reduced (2));
        panicButton.setBounds (h.removeFromRight (70).reduced (4));
    }

    area.removeFromTop (gap);
    pianoBounds = area.removeFromBottom (140);
    if (pianoBar) pianoBar->setBounds (pianoBounds);
    area.removeFromBottom (gap);

    if (mainTab_ == 1)
    {
        settingsPanel.setBounds (area);
        settingsPanel.toFront (false);
        return;
    }

    contentBounds = area.removeFromLeft ((int) (area.getWidth() * 0.27f));
    area.removeFromLeft (gap);
    designBounds = area.removeFromRight ((int) (area.getWidth() * 0.32f));
    area.removeFromRight (gap);
    centerBounds = area;

    browserSection.setBounds (contentBounds);
    if (soundLibrary)
        soundLibrary->setBounds (contentOf (browserSection));

    liveSection.setBounds (centerBounds);
    {
        auto c = contentOf (liveSection);
        auto hero = c.removeFromTop ((int) (c.getHeight() * 0.38f));
        portalStage.setBounds (hero.removeFromLeft ((int) (hero.getWidth() * 0.55f)).reduced (2));
        auto side = hero.reduced (2);
        scope.setBounds (side.removeFromTop ((int) (side.getHeight() * 0.45f)));
        side.removeFromTop (6);
        spectrogram.setBounds (side);

        c.removeFromTop (8);
        auto mid = c.removeFromTop (72);
        wavetablePad.setBounds (mid.removeFromLeft ((int) (mid.getWidth() * 0.45f)).reduced (2));
        meter.setBounds (mid.removeFromLeft ((int) (mid.getWidth() * 0.35f)).reduced (4));
        voiceMeter.setBounds (mid.reduced (2));

        c.removeFromTop (8);
        lfo1Panel.setBounds (c.removeFromTop (150).reduced (2));
        c.removeFromTop (8);
        auto macros = c;
        const int mw = macros.getWidth() / 4;
        macroCard1.setBounds (macros.removeFromLeft (mw).reduced (4));
        macroCard2.setBounds (macros.removeFromLeft (mw).reduced (4));
        macroCard3.setBounds (macros.removeFromLeft (mw).reduced (4));
        macroCard4.setBounds (macros.reduced (4));
    }

    designSection.setBounds (designBounds);
    layoutInspector();

    if (soundLibrary) soundLibrary->toFront (false);
    portalStage.toFront (false);
    scope.toFront (false);
    spectrogram.toFront (false);
    wavetablePad.toFront (false);
    meter.toFront (false);
    voiceMeter.toFront (false);
    lfo1Panel.toFront (false);
    for (auto* m : { &macroCard1, &macroCard2, &macroCard3, &macroCard4 })
        m->toFront (false);
    filterInspector.toFront (false);
    ampInspector.toFront (false);
    fxInspector.toFront (false);
    masterInspector.toFront (false);
}
