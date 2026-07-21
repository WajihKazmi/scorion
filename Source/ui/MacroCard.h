#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"

/** Macro as a mini-card: knob + name + value. */
class MacroCard : public juce::Component
{
public:
    MacroCard()
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob.setName ("hero");
        knob.setComponentID ("modring");
        knob.setPopupDisplayEnabled (true, true, this);
        knob.setDoubleClickReturnValue (true, 0.5);
        addAndMakeVisible (knob);

        title.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (title);
        value.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (value);

        knob.onValueChange = [this] {
            value.setText (knob.getTextFromValue (knob.getValue()), juce::dontSendNotification);
        };
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf != nullptr)
        {
            title.setFont (laf->labelFont());
            title.setColour (juce::Label::textColourId, laf->textSecondary());
            value.setFont (laf->valueFont (10.0f));
            value.setColour (juce::Label::textColourId, laf->ember());
        }
    }

    void setTitle (const juce::String& t) { title.setText (t, juce::dontSendNotification); }
    juce::Slider& getKnob() noexcept { return knob; }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.0f);
        if (laf_ != nullptr)
            laf_->drawCard (g, b, false, isMouseOver (true), 12.0f);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        title.setBounds (a.removeFromTop (14));
        value.setBounds (a.removeFromBottom (14));
        knob.setBounds (a.reduced (4));
    }

private:
    ScorionLookAndFeel* laf_ = nullptr;
    juce::Slider knob;
    juce::Label title, value;
};
