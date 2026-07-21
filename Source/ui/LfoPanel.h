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
            s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setComponentID ("mod");
            s.setName ("mod");
            s.setPopupDisplayEnabled (false, false, nullptr);
            s.setScrollWheelEnabled (true);
            s.setMouseDragSensitivity (160);
            s.setVelocityBasedMode (true);
            s.setVelocityModeParameters (0.8, 1, 0.09, true);
            s.setWantsKeyboardFocus (false);
            s.setTooltip (name);
            s.textFromValueFunction = [] (double v) {
                if (std::abs (v) >= 10.0) return juce::String (v, 1);
                return juce::String (v, 2);
            };

            auto showValue = [&s, &l, this] {
                l.setText (s.getTextFromValue (s.getValue()), juce::dontSendNotification);
                if (laf_ != nullptr)
                {
                    l.setColour (juce::Label::textColourId, laf_->ember());
                    l.setFont (laf_->valueFont (10.0f));
                }
            };
            auto showName = [&l, name, this] {
                l.setText (name, juce::dontSendNotification);
                if (laf_ != nullptr)
                {
                    l.setColour (juce::Label::textColourId, laf_->textSecondary());
                    l.setFont (laf_->labelFont());
                }
                else
                    l.setFont (juce::FontOptions (10.0f));
            };
            s.onDragStart = showValue;
            s.onValueChange = [showValue, &s] { if (s.isMouseButtonDown()) showValue(); };
            s.onDragEnd = showName;

            addAndMakeVisible (s);
            showName();
            l.setJustificationType (juce::Justification::centred);
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
        const int n = 128;
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) (n - 1);
            const float ph = std::fmod (phaseDisp_ + t, 1.0f);
            float v = std::sin (ph * juce::MathConstants<float>::twoPi);
            const int wi = waveBox.getSelectedItemIndex();
            if (wi == 1) v = 2.0f * std::abs (2.0f * ph - 1.0f) - 1.0f; // tri
            else if (wi == 2) v = 2.0f * ph - 1.0f; // saw
            else if (wi == 3) v = ph < 0.5f ? 1.0f : -1.0f; // square
            const float x = scope.getX() + t * scope.getWidth();
            const float y = scope.getCentreY() - v * scope.getHeight() * 0.34f;
            if (i == 0) wave.startNewSubPath (x, y);
            else
            {
                const float t0 = (float) (i - 1) / (float) (n - 1);
                const float x0 = scope.getX() + t0 * scope.getWidth();
                wave.quadraticTo (0.5f * (x0 + x), y, x, y);
            }
        }
        g.setColour ((laf_ != nullptr ? laf_->mint() : juce::Colours::cyan).withAlpha (0.25f));
        g.strokePath (wave, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour ((laf_ != nullptr ? laf_->mint() : juce::Colours::cyan).withAlpha (0.9f));
        g.strokePath (wave, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
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
        phase_ = std::fmod (phase_ + 0.012f, 1.0f);
        phaseDisp_ += 0.16f * (phase_ - phaseDisp_);
        if (std::abs (phase_ - phaseDisp_) > 0.5f)
            phaseDisp_ = phase_;
        repaint (getLocalBounds().removeFromTop (90));
    }

    ScorionLookAndFeel* laf_ = nullptr;
    juce::String title_ { "LFO 1" };
    float phase_ = 0.0f;
    float phaseDisp_ = 0.0f;
};
