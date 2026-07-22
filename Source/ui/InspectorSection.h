#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "Motion.h"

/** Collapsible inspector section with animated open/close. */
class InspectorSection : public juce::Component,
                         private juce::Timer
{
public:
    InspectorSection()
    {
        header.setButtonText ("SECTION");
        header.setClickingTogglesState (true);
        header.setToggleState (true, juce::dontSendNotification);
        header.onClick = [this] {
            targetOpen_ = header.getToggleState() ? 1.0f : 0.0f;
            startTimerHz (24);
        };
        addAndMakeVisible (header);
        addAndMakeVisible (body);
        // Idle: no animation timer — only run while collapsing/expanding
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf != nullptr)
        {
            header.setColour (juce::TextButton::buttonColourId, laf->card());
            header.setColour (juce::TextButton::textColourOffId, laf->textPrimary());
            header.setColour (juce::TextButton::textColourOnId, laf->textPrimary());
        }
    }

    void setTitle (const juce::String& t)
    {
        title_ = t;
        header.setButtonText ((openAmount_ > 0.5f ? "▾  " : "▸  ") + title_);
    }

    juce::Component& getBody() noexcept { return body; }
    float openAmount() const noexcept { return openAmount_; }
    int preferredHeight (int contentH) const
    {
        return 28 + (int) ((float) contentH * openAmount_);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
        {
            g.setColour (laf_->panel());
            g.fillRoundedRectangle (b, 10.0f);
            g.setColour (laf_->border());
            g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f);
        }
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (4);
        header.setBounds (a.removeFromTop (24));
        body.setBounds (a);
        body.setVisible (openAmount_ > 0.05f);
        header.setButtonText ((openAmount_ > 0.5f ? "▾  " : "▸  ") + title_);
    }

private:
    void timerCallback() override
    {
        const float speed = 1.0f / (Motion::panelSec * 60.0f);
        if (std::abs (openAmount_ - targetOpen_) < 0.01f)
        {
            openAmount_ = targetOpen_;
            stopTimer();
        }
        else
            openAmount_ += (targetOpen_ > openAmount_ ? speed : -speed);

        openAmount_ = juce::jlimit (0.0f, 1.0f, openAmount_);
        if (std::abs (openAmount_ - targetOpen_) < 0.001f)
        {
            openAmount_ = targetOpen_;
            stopTimer();
        }
        if (onOpenChanged) onOpenChanged();
        resized();
        repaint();
    }

public:
    std::function<void()> onOpenChanged;

private:
    ScorionLookAndFeel* laf_ = nullptr;
    juce::TextButton header;
    juce::Component body;
    juce::String title_ { "SECTION" };
    float openAmount_ = 1.0f;
    float targetOpen_ = 1.0f;
};
