#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"

/** Labeled inset section with title strip + content padding (Portal-style zoning). */
class SectionPanel : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setTitle (const juce::String& t) { title_ = t; repaint(); }
    void setSubtitle (const juce::String& s) { subtitle_ = s; repaint(); }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }

    juce::Rectangle<int> contentBounds() const
    {
        return getLocalBounds().reduced (14, 12).withTrimmedTop (22);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ == nullptr) return;

        g.setColour (laf_->panel());
        g.fillRoundedRectangle (b, 14.0f);

        // Dynamic gothic rim — breathes with energy
        const float glow = 0.12f + energy_ * 0.22f;
        g.setColour (laf_->border());
        g.drawRoundedRectangle (b.reduced (0.5f), 14.0f, 1.0f);
        g.setColour (laf_->violet().withAlpha (glow));
        g.drawRoundedRectangle (b.reduced (1.5f), 13.0f, 1.2f);
        g.setColour (laf_->ember().withAlpha (0.06f + energy_ * 0.1f));
        g.drawRoundedRectangle (b.reduced (2.5f), 12.0f, 1.0f);

        // Title rail
        auto rail = b.removeFromTop (28.0f).reduced (12.0f, 0.0f);
        g.setColour (laf_->textSecondary());
        g.setFont (laf_->labelFont());
        g.drawText (title_.toUpperCase(), rail.removeFromLeft (rail.getWidth() * 0.55f),
                    juce::Justification::centredLeft, false);
        if (subtitle_.isNotEmpty())
        {
            g.setColour (laf_->mint().withAlpha (0.75f));
            g.setFont (laf_->uiFont (10.0f, true));
            g.drawText (subtitle_, rail, juce::Justification::centredRight, false);
        }

        // Hairline under title
        g.setColour (laf_->violet().withAlpha (0.25f));
        g.fillRect (b.getX() + 12.0f, b.getY(), b.getWidth() - 24.0f, 1.0f);
    }

private:
    ScorionLookAndFeel* laf_ = nullptr;
    juce::String title_ { "SECTION" }, subtitle_;
    float energy_ = 0.0f;
};
