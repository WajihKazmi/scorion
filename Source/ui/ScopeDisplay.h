#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <array>
#include <cmath>

/** Live spectrum with strong smoothing + curved path (less glitch). */
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
            const float tilt = 0.7f + 0.4f * ((float) b / (float) (kBands - 1));
            const float target = juce::jlimit (0.0f, 1.0f, e * 5.8f * tilt);
            // Heavy low-pass — prevents frame-to-frame jitter
            bands_[(size_t) b] += 0.12f * (target - bands_[(size_t) b]);
            peaks_[(size_t) b] = juce::jmax (peaks_[(size_t) b] * 0.985f, bands_[(size_t) b]);
        }
        // Neighbour blur for smoother silhouette
        std::array<float, kBands> blur {};
        for (int i = 0; i < kBands; ++i)
        {
            const float l = bands_[(size_t) juce::jmax (0, i - 1)];
            const float c = bands_[(size_t) i];
            const float r = bands_[(size_t) juce::jmin (kBands - 1, i + 1)];
            blur[(size_t) i] = 0.2f * l + 0.6f * c + 0.2f * r;
        }
        bands_ = blur;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 10.0f);

        auto plot = b.reduced (14.0f, 18.0f);
        const auto ember = laf_ != nullptr ? laf_->ember() : juce::Colour (0xffffffff);
        const auto mint = laf_ != nullptr ? laf_->mint() : juce::Colour (0xff00f0ff);

        g.setColour ((laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey).withAlpha (0.14f));
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
            const float h = bands_[(size_t) i] * plot.getHeight() * 0.88f;
            const float y = plot.getBottom() - h;
            if (i == 0)
            {
                fill.startNewSubPath (x, plot.getBottom());
                fill.lineTo (x, y);
                line.startNewSubPath (x, y);
            }
            else
            {
                const float t0 = (float) (i - 1) / (float) (kBands - 1);
                const float x0 = plot.getX() + t0 * plot.getWidth();
                const float y0 = plot.getBottom() - bands_[(size_t) (i - 1)] * plot.getHeight() * 0.88f;
                const float mx = 0.5f * (x0 + x);
                fill.quadraticTo (mx, y0, x, y);
                line.quadraticTo (mx, y0, x, y);
            }
        }
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.closeSubPath();

        juce::ColourGradient grad (ember.withAlpha (0.35f + energy_ * 0.18f), plot.getCentreX(), plot.getY(),
                                   mint.withAlpha (0.04f), plot.getCentreX(), plot.getBottom(), false);
        g.setGradientFill (grad);
        g.fillPath (fill);

        g.setColour (ember.withAlpha (0.22f + energy_ * 0.2f));
        g.strokePath (line, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colours::white.withAlpha (0.82f));
        g.strokePath (line, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::ignoreUnused (phase_);
    }

private:
    static constexpr int kBands = 48;
    ScorionLookAndFeel* laf_ = nullptr;
    std::array<float, kBands> bands_ {};
    std::array<float, kBands> peaks_ {};
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
