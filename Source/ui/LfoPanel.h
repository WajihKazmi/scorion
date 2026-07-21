#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <cmath>
#include <array>

/** Inline LFO strip with live waveform (purple neon accent). */
class LfoPanel : public juce::Component,
                 private juce::Timer
{
public:
    LfoPanel()
    {
        auto styleKnob = [this] (juce::Slider& s, juce::Label& l, const juce::String& name) {
            s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setComponentID ("mod");
            s.setName ("mod");
            addAndMakeVisible (s);
            l.setText (name, juce::dontSendNotification);
            l.setJustificationType (juce::Justification::centred);
            l.setFont (juce::FontOptions (10.0f));
            addAndMakeVisible (l);
        };

        waveBox.addItemList ({ "Sine", "Tri", "Saw", "Square", "S&H", "Smooth" }, 1);
        waveBox.setSelectedId (1);
        addAndMakeVisible (waveBox);
        syncButton.setClickingTogglesState (true);
        addAndMakeVisible (syncButton);
        divBox.addItemList ({ "1/16", "1/8", "1/4", "1/2", "1 bar", "2 bars" }, 1);
        divBox.setSelectedId (3);
        addAndMakeVisible (divBox);

        styleKnob (rateSlider, rateLabel, "Rate");
        styleKnob (cutSlider, cutLabel, "Cutoff");
        styleKnob (wtSlider, wtLabel, "WT");
        styleKnob (panSlider, panLabel, "Pan");
        styleKnob (ampSlider, ampLabel, "Amp");

        rateSlider.setRange (0.05, 30.0, 0.01);
        cutSlider.setRange (-1.0, 1.0, 0.001);
        wtSlider.setRange (-1.0, 1.0, 0.001);
        panSlider.setRange (-1.0, 1.0, 0.001);
        ampSlider.setRange (-1.0, 1.0, 0.001);

        startTimerHz (30);
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setTitle (const juce::String& t) { title_ = t; repaint(); }
    void setPhase (float p) { phase_ = p; }

    juce::ComboBox waveBox;
    juce::TextButton syncButton { "Sync" };
    juce::ComboBox divBox;
    juce::Slider rateSlider, cutSlider, wtSlider, panSlider, ampSlider;
    juce::Label rateLabel, cutLabel, wtLabel, panLabel, ampLabel;

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawPanel (g, b, 12.0f);

        auto head = b.removeFromTop (28.0f).reduced (12.0f, 6.0f);
        g.setColour (laf_ != nullptr ? laf_->mint() : juce::Colour (0xff5ee1a4));
        g.setFont (laf_ != nullptr ? laf_->uiFont (12.0f, true) : juce::FontOptions (12.0f));
        g.drawText (title_, head, juce::Justification::centredLeft);

        auto scope = b.removeFromTop (48.0f).reduced (12.0f, 4.0f);
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, scope, 8.0f);

        juce::Path wave;
        const int n = 64;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float ph = std::fmod (phase_ + t, 1.0f);
            float v = std::sin (ph * juce::MathConstants<float>::twoPi);
            const float x = scope.getX() + t * scope.getWidth();
            const float y = scope.getCentreY() - v * scope.getHeight() * 0.35f;
            if (i == 0) wave.startNewSubPath (x, y);
            else wave.lineTo (x, y);
        }
        g.setColour ((laf_ != nullptr ? laf_->mint() : juce::Colours::green).withAlpha (0.85f));
        g.strokePath (wave, juce::PathStrokeType (1.6f));
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (10);
        a.removeFromTop (28);
        a.removeFromTop (52);
        auto row = a.removeFromTop (28);
        waveBox.setBounds (row.removeFromLeft (90).reduced (2));
        syncButton.setBounds (row.removeFromLeft (56).reduced (2));
        divBox.setBounds (row.removeFromLeft (80).reduced (2));

        auto knobs = a;
        const int w = knobs.getWidth() / 5;
        auto place = [&] (juce::Slider& s, juce::Label& l) {
            auto c = knobs.removeFromLeft (w).reduced (2);
            l.setBounds (c.removeFromBottom (14));
            s.setBounds (c);
        };
        place (rateSlider, rateLabel);
        place (cutSlider, cutLabel);
        place (wtSlider, wtLabel);
        place (panSlider, panLabel);
        place (ampSlider, ampLabel);
    }

private:
    void timerCallback() override
    {
        phase_ = std::fmod (phase_ + 0.02f, 1.0f);
        repaint (getLocalBounds().removeFromTop (90));
    }

    ScorionLookAndFeel* laf_ = nullptr;
    juce::String title_ { "LFO 1" };
    float phase_ = 0.0f;
};
