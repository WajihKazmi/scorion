#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"

/** Macro as a mini-card: knob + name + one value readout. */
class MacroCard : public juce::Component
{
public:
    MacroCard()
    {
        knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        knob.setName ("hero");
        knob.setComponentID ("modring");
        knob.setPopupDisplayEnabled (false, false, nullptr); // value label below is the only readout
        knob.setScrollWheelEnabled (true);
        knob.setMouseDragSensitivity (160);
        knob.setVelocityBasedMode (true);
        knob.setVelocityModeParameters (0.8, 1, 0.09, true);
        knob.setDoubleClickReturnValue (true, 0.5);
        knob.setWantsKeyboardFocus (false);
        knob.setTooltip ("Macro");
        knob.textFromValueFunction = [] (double v) { return juce::String (v, 3); };
        addAndMakeVisible (knob);

        title.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (title);
        value.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (value);

        auto syncValue = [this] {
            value.setText (knob.getTextFromValue (knob.getValue()), juce::dontSendNotification);
            const bool hot = knob.isMouseButtonDown() || knob.isMouseOver();
            if (laf_ != nullptr)
                value.setColour (juce::Label::textColourId,
                                 hot ? laf_->ember() : laf_->textSecondary());
        };
        knob.onValueChange = syncValue;
        knob.onDragStart = syncValue;
        knob.onDragEnd = syncValue;
        syncValue();
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf != nullptr)
        {
            title.setFont (laf->labelFont());
            title.setColour (juce::Label::textColourId, laf->textSecondary());
            value.setFont (laf->valueFont (11.0f));
            value.setColour (juce::Label::textColourId, laf->textSecondary());
        }
    }

    void setTitle (const juce::String& t) { title.setText (t, juce::dontSendNotification); }
    juce::Slider& getKnob() noexcept { return knob; }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (2.0f);
        if (laf_ != nullptr)
            laf_->drawCard (g, b, false, isMouseOver (true) || knob.isMouseOverOrDragging(), 12.0f);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        title.setBounds (a.removeFromTop (14));
        value.setBounds (a.removeFromBottom (16));
        knob.setBounds (a.reduced (4));
    }

private:
    ScorionLookAndFeel* laf_ = nullptr;
    juce::Slider knob;
    juce::Label title, value;
};
