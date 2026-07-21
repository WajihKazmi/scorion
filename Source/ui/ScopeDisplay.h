#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <array>
#include <cmath>

/** Live spectrum — crimson fill, purple neon peak ribbon. */
class ScopeDisplay : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setAnimPhase (float p) { phase_ = p; }

    void setSnapshot (const std::array<float, 256>& data)
    {
        for (int b = 0; b < kBands; ++b)
        {
            const int start = b * 256 / kBands;
            const int end = (b + 1) * 256 / kBands;
            float e = 0.0f;
            for (int i = start; i < end; ++i)
                e += data[(size_t) i] * data[(size_t) i];
            e = std::sqrt (e / (float) juce::jmax (1, end - start));
            const float tilt = 0.65f + 0.55f * ((float) b / (float) (kBands - 1));
            const float shimmer = 0.015f + 0.03f * (0.5f + 0.5f * std::sin (phase_ + (float) b * 0.15f));
            const float target = juce::jlimit (0.0f, 1.0f, e * 6.5f * tilt + shimmer * (0.3f + energy_));
            bands_[(size_t) b] += 0.28f * (target - bands_[(size_t) b]);
            peaks_[(size_t) b] = juce::jmax (peaks_[(size_t) b] * 0.96f, bands_[(size_t) b]);
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 10.0f);

        auto plot = b.reduced (14.0f, 18.0f);
        const auto ember = laf_ != nullptr ? laf_->ember() : juce::Colour (0xffff6a3d);
        const auto mint = laf_ != nullptr ? laf_->mint() : juce::Colour (0xff5ee1a4);

        g.setColour ((laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey).withAlpha (0.2f));
        for (int i = 0; i < 5; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / 4.0f;
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        }

        juce::Path fill, line;
        for (int i = 0; i < kBands; ++i)
        {
            const float t = (float) i / (float) (kBands - 1);
            const float x = plot.getX() + t * plot.getWidth();
            const float h = bands_[(size_t) i] * plot.getHeight() * 0.9f;
            const float y = plot.getBottom() - h;
            if (i == 0) { fill.startNewSubPath (x, plot.getBottom()); fill.lineTo (x, y); line.startNewSubPath (x, y); }
            else { fill.lineTo (x, y); line.lineTo (x, y); }
        }
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.closeSubPath();

        juce::ColourGradient grad (ember.withAlpha (0.45f + energy_ * 0.2f), plot.getCentreX(), plot.getY(),
                                   mint.withAlpha (0.05f), plot.getCentreX(), plot.getBottom(), false);
        g.setGradientFill (grad);
        g.fillPath (fill);
        g.setColour (ember.withAlpha (0.35f + energy_ * 0.3f));
        g.strokePath (line, juce::PathStrokeType (3.5f));
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.strokePath (line, juce::PathStrokeType (1.4f));

        juce::Path peak;
        for (int i = 0; i < kBands; ++i)
        {
            const float t = (float) i / (float) (kBands - 1);
            const float x = plot.getX() + t * plot.getWidth();
            const float y = plot.getBottom() - peaks_[(size_t) i] * plot.getHeight() * 0.9f;
            if (i == 0) peak.startNewSubPath (x, y);
            else peak.lineTo (x, y);
        }
        g.setColour (mint.withAlpha (0.45f));
        g.strokePath (peak, juce::PathStrokeType (1.0f));
    }

private:
    static constexpr int kBands = 64;
    ScorionLookAndFeel* laf_ = nullptr;
    std::array<float, kBands> bands_ {};
    std::array<float, kBands> peaks_ {};
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
