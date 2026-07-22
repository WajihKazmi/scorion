#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "audio/PerformanceMode.h"
#include <functional>

/** Settings tab: themes, UI scale, library + FL Studio options. */
class SettingsPanel : public juce::Component
{
public:
    SettingsPanel()
    {
        title.setText ("SETTINGS", juce::dontSendNotification);
        title.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (title);

        themeLabel.setText ("PREMIUM THEMES", juce::dontSendNotification);
        addAndMakeVisible (themeLabel);

        for (int i = 0; i < ScorionLookAndFeel::kNumThemes; ++i)
        {
            auto theme = (ScorionLookAndFeel::NamedTheme) i;
            auto* b = themeButtons.add (new juce::TextButton (ScorionLookAndFeel::themeName (theme)));
            b->setClickingTogglesState (true);
            b->setRadioGroupId (7701);
            b->setTooltip (ScorionLookAndFeel::themeBlurb (theme));
            b->onClick = [this, theme] {
                if (onThemeChanged) onThemeChanged (theme);
                refreshThemeToggles();
            };
            addAndMakeVisible (b);
        }

        scaleLabel.setText ("UI SCALE", juce::dontSendNotification);
        addAndMakeVisible (scaleLabel);
        scaleBox.addItemList ({ "85%", "100%", "110%", "125%", "150%" }, 1);
        scaleBox.setSelectedId (2, juce::dontSendNotification);
        scaleBox.onChange = [this] {
            static const float scales[] = { 0.85f, 1.0f, 1.1f, 1.25f, 1.5f };
            const int i = juce::jlimit (0, 4, scaleBox.getSelectedItemIndex());
            if (onUiScaleChanged) onUiScaleChanged (scales[i]);
        };
        addAndMakeVisible (scaleBox);

        perfLabel.setText ("PERFORMANCE (CPU / RAM)", juce::dontSendNotification);
        addAndMakeVisible (perfLabel);
        perfBox.addItemList ({ "Eco (low CPU / RAM)", "Balanced", "Quality" }, 1);
        perfBox.setSelectedId (1, juce::dontSendNotification); // Eco default
        perfBox.setTooltip ("Eco targets older laptops (e.g. 6th-gen). Fewer voices, lighter FX & UI.");
        perfBox.onChange = [this] {
            const auto mode = (PerformanceMode) juce::jlimit (0, 2, perfBox.getSelectedItemIndex());
            if (onPerformanceChanged) onPerformanceChanged (mode);
        };
        addAndMakeVisible (perfBox);

        knobScales.setButtonText ("Show knob scale numbers (0–10)");
        knobScales.setClickingTogglesState (true);
        knobScales.setToggleState (true, juce::dontSendNotification);
        knobScales.onClick = [this] {
            if (onKnobScalesChanged) onKnobScalesChanged (knobScales.getToggleState());
        };
        addAndMakeVisible (knobScales);

        audition.setButtonText ("Audition preset on library click");
        audition.setClickingTogglesState (true);
        audition.setToggleState (true, juce::dontSendNotification);
        audition.onClick = [this] {
            if (onAuditionChanged) onAuditionChanged (audition.getToggleState());
        };
        addAndMakeVisible (audition);

        flFriendly.setButtonText ("FL Studio friendly mode (programs + focus)");
        flFriendly.setClickingTogglesState (true);
        flFriendly.setToggleState (true, juce::dontSendNotification);
        flFriendly.onClick = [this] {
            if (onFlFriendlyChanged) onFlFriendlyChanged (flFriendly.getToggleState());
        };
        addAndMakeVisible (flFriendly);

        midiChLabel.setText ("PLUGIN MIDI CHANNEL", juce::dontSendNotification);
        addAndMakeVisible (midiChLabel);
        midiChannel.addItem ("Omni (all)", 1);
        for (int c = 1; c <= 16; ++c)
            midiChannel.addItem ("Ch " + juce::String (c), c + 1);
        midiChannel.setSelectedId (1, juce::dontSendNotification);
        midiChannel.onChange = [this] {
            const int id = midiChannel.getSelectedId();
            if (onMidiChannelChanged) onMidiChannelChanged (id <= 1 ? 0 : id - 1);
        };
        addAndMakeVisible (midiChannel);

        tip.setText ("Tip: In FL Studio, load Scorion as VST3. Use Wrapper settings → "
                     "\"Use fixed size\" off for DPI scaling. Presets appear as host programs.",
                     juce::dontSendNotification);
        tip.setJustificationType (juce::Justification::topLeft);
        addAndMakeVisible (tip);
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf == nullptr) return;
        title.setFont (laf->brandFont (18.0f));
        title.setColour (juce::Label::textColourId, laf->textPrimary());
        for (auto* lab : { &themeLabel, &scaleLabel, &perfLabel, &midiChLabel })
        {
            lab->setFont (laf->titleFont (12.0f));
            lab->setColour (juce::Label::textColourId, laf->textPrimary());
        }
        tip.setFont (laf->uiFont (12.0f));
        tip.setColour (juce::Label::textColourId, laf->textSecondary());
        refreshThemeToggles();
        repaint();
    }

    void syncFromLookAndFeel (ScorionLookAndFeel& laf)
    {
        refreshThemeToggles();
        knobScales.setToggleState (laf.getShowKnobScales(), juce::dontSendNotification);
        const float s = laf.getUiScale();
        int id = 2;
        if (s < 0.9f) id = 1;
        else if (s < 1.05f) id = 2;
        else if (s < 1.18f) id = 3;
        else if (s < 1.35f) id = 4;
        else id = 5;
        scaleBox.setSelectedId (id, juce::dontSendNotification);
    }

    void setAudition (bool on) { audition.setToggleState (on, juce::dontSendNotification); }
    void setFlFriendly (bool on) { flFriendly.setToggleState (on, juce::dontSendNotification); }
    void setPerformanceMode (PerformanceMode m)
    {
        perfBox.setSelectedItemIndex ((int) m, juce::dontSendNotification);
    }

    std::function<void (ScorionLookAndFeel::NamedTheme)> onThemeChanged;
    std::function<void (float)> onUiScaleChanged;
    std::function<void (bool)> onKnobScalesChanged;
    std::function<void (bool)> onAuditionChanged;
    std::function<void (bool)> onFlFriendlyChanged;
    std::function<void (int)> onMidiChannelChanged;
    std::function<void (PerformanceMode)> onPerformanceChanged;

    void paint (juce::Graphics& g) override
    {
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, getLocalBounds().toFloat(), 12.0f);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (16);
        title.setBounds (a.removeFromTop (28));
        a.removeFromTop (12);
        themeLabel.setBounds (a.removeFromTop (18));
        a.removeFromTop (6);
        for (auto* b : themeButtons)
        {
            b->setBounds (a.removeFromTop (36).reduced (0, 3));
        }
        a.removeFromTop (16);
        scaleLabel.setBounds (a.removeFromTop (18));
        a.removeFromTop (4);
        scaleBox.setBounds (a.removeFromTop (32));
        a.removeFromTop (12);
        perfLabel.setBounds (a.removeFromTop (18));
        a.removeFromTop (4);
        perfBox.setBounds (a.removeFromTop (32));
        a.removeFromTop (12);
        knobScales.setBounds (a.removeFromTop (28));
        audition.setBounds (a.removeFromTop (28));
        flFriendly.setBounds (a.removeFromTop (28));
        a.removeFromTop (12);
        midiChLabel.setBounds (a.removeFromTop (18));
        a.removeFromTop (4);
        midiChannel.setBounds (a.removeFromTop (32));
        a.removeFromTop (16);
        tip.setBounds (a.removeFromTop (72));
    }

private:
    void refreshThemeToggles()
    {
        if (laf_ == nullptr) return;
        const int cur = (int) laf_->getNamedTheme();
        for (int i = 0; i < themeButtons.size(); ++i)
            themeButtons[i]->setToggleState (i == cur, juce::dontSendNotification);
    }

    ScorionLookAndFeel* laf_ = nullptr;
    juce::Label title, themeLabel, scaleLabel, perfLabel, midiChLabel, tip;
    juce::OwnedArray<juce::TextButton> themeButtons;
    juce::ComboBox scaleBox, midiChannel, perfBox;
    juce::ToggleButton knobScales, audition, flFriendly;
};
