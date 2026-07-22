#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <cmath>
#include <vector>

/** Pseudo-3D wavetable / harmonic table visualization. */
class WavetablePad : public juce::Component,
                     private juce::Timer
{
public:
    WavetablePad()
    {
        buildFrames();
        startTimerHz (24);
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setPosition (float pos01) { position_ = juce::jlimit (0.0f, 1.0f, pos01); }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setTimerHz (int hz)
    {
        reducedLayers_ = hz <= 16;
        startTimerHz (juce::jlimit (6, 30, hz));
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 12.0f);

        auto plot = b.reduced (14.0f, 16.0f);
        const auto accent = laf_ != nullptr ? laf_->accent() : juce::Colours::cyan;
        const int layers = reducedLayers_ ? 4 : 7;
        const float frameF = position_ * (float) (numFrames_ - 1);
        const int f0 = (int) frameF;
        const int f1 = juce::jmin (f0 + 1, numFrames_ - 1);
        const float frac = frameF - (float) f0;

        for (int layer = layers - 1; layer >= 0; --layer)
        {
            const float t = (float) layer / (float) (layers - 1);
            const float depth = 1.0f - t;
            const float yOffset = t * plot.getHeight() * 0.55f;
            const float xInset = t * plot.getWidth() * 0.12f;
            const float alpha = 0.15f + depth * 0.55f;
            const float morph = juce::jlimit (0.0f, (float) numFrames_ - 1.001f,
                                              frameF + (float) layer * 0.15f + phase_ * 0.3f);

            juce::Path p;
            const int points = 96;
            for (int i = 0; i < points; ++i)
            {
                const float u = (float) i / (float) (points - 1);
                const float s = sampleMorph (morph, u) * (0.55f + 0.45f * energy_);
                const float x = plot.getX() + xInset + u * (plot.getWidth() - 2.0f * xInset);
                const float y = plot.getBottom() - yOffset - (s * 0.5f + 0.5f) * (plot.getHeight() * 0.42f * depth);
                if (i == 0) p.startNewSubPath (x, y);
                else p.lineTo (x, y);
            }

            g.setColour (accent.withAlpha (alpha * 0.35f));
            g.strokePath (p, juce::PathStrokeType (4.0f));
            g.setColour (accent.interpolatedWith (juce::Colours::white, depth * 0.25f).withAlpha (alpha));
            g.strokePath (p, juce::PathStrokeType (1.4f));
        }

        // Floor grid perspective
        g.setColour (accent.withAlpha (0.08f));
        for (int i = 0; i < 6; ++i)
        {
            const float t = (float) i / 5.0f;
            const float y = plot.getY() + plot.getHeight() * (0.35f + t * 0.65f);
            const float inset = (1.0f - t) * plot.getWidth() * 0.1f;
            g.drawLine (plot.getX() + inset, y, plot.getRight() - inset, y, 1.0f);
        }

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (juce::FontOptions (11.0f));
        g.drawText ("WAVETABLE", plot.getX() + 2.0f, plot.getY() - 4.0f, 90.0f, 14.0f,
                    juce::Justification::centredLeft);

        g.setColour (accent.withAlpha (0.85f));
        g.drawText (juce::String (position_ * 100.0f, 0) + "%",
                    plot.getRight() - 48.0f, plot.getY() - 4.0f, 48.0f, 14.0f,
                    juce::Justification::centredRight);

        juce::ignoreUnused (f0, f1, frac);
    }

private:
    void timerCallback() override
    {
        phase_ += 0.012f + energy_ * 0.02f;
        if (phase_ > juce::MathConstants<float>::twoPi)
            phase_ -= juce::MathConstants<float>::twoPi;
        repaint();
    }

    void buildFrames()
    {
        frameSize_ = 128;
        numFrames_ = 8;
        table_.assign ((size_t) (frameSize_ * numFrames_), 0.0f);
        for (int f = 0; f < numFrames_; ++f)
        {
            const int harmonics = 1 + f * 2;
            for (int i = 0; i < frameSize_; ++i)
            {
                float s = 0.0f;
                const float ph = (float) i / (float) frameSize_;
                for (int h = 1; h <= harmonics; ++h)
                    s += std::sin (ph * juce::MathConstants<float>::twoPi * (float) h) / (float) h;
                table_[(size_t) (f * frameSize_ + i)] = juce::jlimit (-1.0f, 1.0f, s * 0.55f);
            }
        }
    }

    float sampleMorph (float framePos, float phase01) const
    {
        const float fPos = juce::jlimit (0.0f, (float) numFrames_ - 1.001f, framePos);
        const int f0 = (int) fPos;
        const int f1 = juce::jmin (f0 + 1, numFrames_ - 1);
        const float fracF = fPos - (float) f0;
        const float idx = phase01 * (float) (frameSize_ - 1);
        const int i0 = (int) idx;
        const int i1 = juce::jmin (i0 + 1, frameSize_ - 1);
        const float frac = idx - (float) i0;
        const float a = table_[(size_t) (f0 * frameSize_ + i0)] * (1.0f - frac)
                      + table_[(size_t) (f0 * frameSize_ + i1)] * frac;
        const float b = table_[(size_t) (f1 * frameSize_ + i0)] * (1.0f - frac)
                      + table_[(size_t) (f1 * frameSize_ + i1)] * frac;
        return a + (b - a) * fracF;
    }

    ScorionLookAndFeel* laf_ = nullptr;
    std::vector<float> table_;
    int frameSize_ = 128;
    int numFrames_ = 8;
    float position_ = 0.25f;
    float energy_ = 0.5f;
    float phase_ = 0.0f;
    bool reducedLayers_ = true;
};
