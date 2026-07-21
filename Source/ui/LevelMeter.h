#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <cmath>

/** Vertical LED meter + LUFS-style readout with pulse glow. */
class LevelMeter : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }

    void setLevel (float linear01)
    {
        level_ += 0.35f * (juce::jlimit (0.0f, 1.0f, linear01) - level_);
        peak_ = juce::jmax (peak_ * 0.985f, level_);
        repaint();
    }

    void setPulse (float p)
    {
        pulse_ += 0.3f * (juce::jlimit (0.0f, 1.0f, p) - pulse_);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (4.0f);
        auto meter = b.removeFromLeft (18.0f);

        // Outer pulse glow
        if (pulse_ > 0.05f)
        {
            const auto accent = laf_ != nullptr ? laf_->accent() : juce::Colour (0xff2fd4c8);
            g.setColour (accent.withAlpha (0.08f + pulse_ * 0.22f));
            g.fillRoundedRectangle (meter.expanded (4.0f + pulse_ * 3.0f), 8.0f);
        }

        g.setColour (juce::Colour (0xff010003));
        g.fillRoundedRectangle (meter, 6.0f);

        const int segs = 24;
        const float gap = 2.0f;
        const float segH = (meter.getHeight() - gap * (float) (segs - 1)) / (float) segs;
        const int lit = (int) std::round (level_ * (float) segs);
        const auto accent = laf_ != nullptr ? laf_->accent() : juce::Colour (0xff2fd4c8);
        const float twinkle = 0.5f + 0.5f * std::sin (juce::Time::getMillisecondCounterHiRes() * 0.008);

        for (int i = 0; i < segs; ++i)
        {
            const float y = meter.getBottom() - (float) (i + 1) * (segH + gap) + gap;
            const bool on = i < lit;
            const float t = (float) i / (float) (segs - 1);
            auto c = accent.interpolatedWith (juce::Colours::white, t * 0.25f);
            if (i > segs - 4) c = juce::Colour (0xffff6b7a);
            const float a = on ? (0.85f + 0.15f * twinkle * pulse_) : 0.12f;
            g.setColour (on ? c.withAlpha (a) : c.withAlpha (0.12f));
            g.fillRoundedRectangle (meter.getX() + 3.0f, y, meter.getWidth() - 6.0f, segH, 2.0f);
        }

        // Peak tick
        if (peak_ > 0.02f)
        {
            const float py = meter.getBottom() - peak_ * meter.getHeight();
            g.setColour (juce::Colours::white.withAlpha (0.55f + pulse_ * 0.35f));
            g.fillRect (meter.getX() + 1.0f, py - 1.0f, meter.getWidth() - 2.0f, 2.0f);
        }

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (juce::FontOptions (11.0f));
        const float db = level_ > 1.0e-4f ? 20.0f * std::log10 (level_) : -60.0f;
        const float lufs = -14.0f + db * 0.55f; // display-only
        g.drawText (juce::String (db, 1) + " dB", b.removeFromTop (18.0f), juce::Justification::centredLeft);
        g.setColour (accent.withAlpha (0.75f + pulse_ * 0.25f));
        g.setFont (juce::FontOptions (16.0f).withStyle ("Medium"));
        g.drawText (juce::String (lufs, 1) + " LUFS", b.removeFromTop (22.0f), juce::Justification::centredLeft);
    }

private:
    ScorionLookAndFeel* laf_ = nullptr;
    float level_ = 0.0f;
    float peak_ = 0.0f;
    float pulse_ = 0.0f;
};
