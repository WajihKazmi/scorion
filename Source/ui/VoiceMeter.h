#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"

/** Active-voice pips + energy readout. */
class VoiceMeter : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setVoiceCount (int n) { voices_ = juce::jlimit (0, maxVoices_, n); repaint(); }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 10.0f);

        auto plot = b.reduced (10.0f, 12.0f);
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("VOICES", plot.getX(), plot.getY() - 2.0f, 50.0f, 12.0f, juce::Justification::centredLeft);

        g.setColour (laf_ != nullptr ? laf_->ember() : juce::Colours::red);
        g.setFont (laf_ != nullptr ? laf_->valueFont (11.0f) : juce::FontOptions (11.0f));
        g.drawText (juce::String (voices_) + " / " + juce::String (maxVoices_),
                    plot.getRight() - 48.0f, plot.getY() - 2.0f, 48.0f, 12.0f, juce::Justification::centredRight);

        auto row = plot.withTrimmedTop (14.0f);
        const float gap = 3.0f;
        const float w = (row.getWidth() - gap * (float) (maxVoices_ - 1)) / (float) maxVoices_;
        for (int i = 0; i < maxVoices_; ++i)
        {
            auto pip = juce::Rectangle<float> (row.getX() + (float) i * (w + gap), row.getCentreY() - 5.0f, w, 10.0f);
            const bool on = i < voices_;
            g.setColour (on ? (laf_ != nullptr ? laf_->ember() : juce::Colours::red)
                                 .withAlpha (0.55f + energy_ * 0.4f)
                            : (laf_ != nullptr ? laf_->card() : juce::Colour (0xff221A26)));
            g.fillRoundedRectangle (pip, 3.0f);
        }
    }

private:
    ScorionLookAndFeel* laf_ = nullptr;
    int voices_ = 0;
    int maxVoices_ = 16;
    float energy_ = 0.0f;
};
