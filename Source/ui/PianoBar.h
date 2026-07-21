#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "ScorionLookAndFeel.h"
#include <cmath>

class PianoBar : public juce::Component
{
public:
    PianoBar (juce::MidiKeyboardState& state)
        : keyboard (state, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        keyboard.setAvailableRange (36, 96);
        keyboard.setOctaveForMiddleC (4);
        keyboard.setScrollButtonsVisible (false);
        keyboard.setKeyWidth (18.0f);
        keyboard.setBlackNoteLengthProportion (0.64f);
        keyboard.setBlackNoteWidthProportion (0.7f);
        addAndMakeVisible (keyboard);
    }

    juce::MidiKeyboardComponent& getKeyboard() noexcept { return keyboard; }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf_ != nullptr)
            keyboard.setLookAndFeel (laf_);
    }

    void setEnergy (float e)
    {
        energy_ += 0.25f * (juce::jlimit (0.0f, 1.0f, e) - energy_);
        if (std::abs (energy_ - lastPaintEnergy_) > 0.02f)
        {
            lastPaintEnergy_ = energy_;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawPanel (g, b, 12.0f);

        if (energy_ > 0.02f)
        {
            const auto ember = laf_ != nullptr ? laf_->ember() : juce::Colour (0xffff6a3d);
            g.setColour (ember.withAlpha (0.08f + energy_ * 0.22f));
            g.drawRoundedRectangle (b.reduced (1.5f), 12.0f, 1.5f + energy_ * 2.0f);
        }

        auto header = b.removeFromTop (24.0f).reduced (14.0f, 5.0f);
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("KEYBOARD", header, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->ember() : juce::Colours::orange);
        g.drawText ("A–K computer keys", header, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12);
        area.removeFromTop (22);
        keyboard.setBounds (area.reduced (2));
        const float w = (float) area.getWidth() / 36.0f;
        keyboard.setKeyWidth (juce::jlimit (12.0f, 28.0f, w));
    }

private:
    juce::MidiKeyboardComponent keyboard;
    ScorionLookAndFeel* laf_ = nullptr;
    float energy_ = 0.0f;
    float lastPaintEnergy_ = -1.0f;
};
