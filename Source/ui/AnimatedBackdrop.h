#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <cmath>

/** Live mesh / aurora backdrop with smooth animated gradients. */
class AnimatedBackdrop : public juce::Component,
                         private juce::Timer
{
public:
    AnimatedBackdrop() { startTimerHz (28); setInterceptsMouseClicks (false, false); }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setReducedMotion (bool v) { reduced_ = v; }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const auto deep = laf_ != nullptr ? laf_->background() : juce::Colour (0xff080a10);
        const auto accent = laf_ != nullptr ? laf_->accent() : juce::Colour (0xff2fd4c8);
        const auto blue = laf_ != nullptr ? laf_->accentBlue() : juce::Colour (0xff4d7cff);
        const auto aurora = laf_ != nullptr ? laf_->violet() : juce::Colour (0xffc45dff);

        g.fillAll (deep);

        juce::ColourGradient base (deep.brighter (0.04f), 0, 0, deep.darker (0.05f),
                                   b.getRight(), b.getBottom(), false);
        g.setGradientFill (base);
        g.fillRect (b);

        if (reduced_)
            return;

        const float t = phase_;
        const float e = energy_;
        auto blob = [&] (float cx, float cy, float r, juce::Colour c, float a) {
            g.setColour (c.withAlpha (a));
            g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
        };

        blob (b.getWidth() * (0.18f + 0.04f * std::sin (t * 0.7f)),
              b.getHeight() * (0.22f + 0.05f * std::cos (t * 0.5f)),
              220.0f + e * 80.0f, accent, 0.05f + e * 0.08f);

        blob (b.getWidth() * (0.78f + 0.03f * std::cos (t * 0.55f)),
              b.getHeight() * (0.35f + 0.06f * std::sin (t * 0.4f)),
              260.0f + e * 60.0f, blue, 0.04f + e * 0.07f);

        if (laf_ != nullptr && laf_->getNamedTheme() == ScorionLookAndFeel::NamedTheme::NoirPhosphor)
            blob (b.getWidth() * (0.5f + 0.08f * std::sin (t * 0.35f)),
                  b.getHeight() * 0.7f,
                  300.0f, aurora, 0.035f + e * 0.05f);

        // Soft top vignette curve
        juce::Path veil;
        veil.startNewSubPath (0, 0);
        veil.lineTo (b.getRight(), 0);
        veil.lineTo (b.getRight(), 90.0f);
        veil.quadraticTo (b.getCentreX(), 140.0f + 20.0f * std::sin (t), 0, 90.0f);
        veil.closeSubPath();
        g.setColour (juce::Colours::black.withAlpha (0.22f));
        g.fillPath (veil);
    }

private:
    void timerCallback() override
    {
        if (! reduced_)
            phase_ += 0.028f;
        if (phase_ > juce::MathConstants<float>::twoPi)
            phase_ -= juce::MathConstants<float>::twoPi;
        repaint();
    }

    ScorionLookAndFeel* laf_ = nullptr;
    float phase_ = 0.0f;
    float energy_ = 0.0f;
    bool reduced_ = false;
};
