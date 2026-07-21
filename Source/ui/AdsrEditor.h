#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <functional>
#include <cmath>

/** Animated ADSR curve with glow scan — never static. */
class AdsrEditor : public juce::Component,
                   private juce::Timer
{
public:
    AdsrEditor() { startTimerHz (30); }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setValues (float a, float d, float s, float r)
    {
        attack_ = juce::jlimit (0.001f, 4.0f, a);
        decay_ = juce::jlimit (0.001f, 4.0f, d);
        sustain_ = juce::jlimit (0.0f, 1.0f, s);
        release_ = juce::jlimit (0.001f, 8.0f, r);
        repaint();
    }

    std::function<void (float a, float d, float s, float r)> onChanged;

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 12.0f);

        auto plot = b.reduced (12.0f, 16.0f);
        const float total = attack_ + decay_ + 0.35f + release_;
        const float ax = plot.getX();
        const float peakX = ax + (attack_ / total) * plot.getWidth();
        const float susX = peakX + (decay_ / total) * plot.getWidth();
        const float relX = susX + (0.35f / total) * plot.getWidth();
        const float endX = plot.getRight();
        const float bottom = plot.getBottom();
        const float top = plot.getY() + 4.0f;
        const float susY = bottom - sustain_ * (bottom - top);

        // Grid
        g.setColour ((laf_ != nullptr ? laf_->violet() : juce::Colours::purple).withAlpha (0.15f));
        for (int i = 0; i < 4; ++i)
        {
            const float y = plot.getY() + plot.getHeight() * (float) i / 3.0f;
            g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        }

        juce::Path p;
        p.startNewSubPath (ax, bottom);
        p.lineTo (peakX, top);
        p.lineTo (susX, susY);
        p.lineTo (relX, susY);
        p.lineTo (endX, bottom);

        const auto col = laf_ != nullptr ? laf_->ember() : juce::Colours::orange;
        const auto mint = laf_ != nullptr ? laf_->mint() : juce::Colours::purple;

        auto fill = p;
        fill.lineTo (ax, bottom);
        fill.closeSubPath();
        juce::ColourGradient grad (col.withAlpha (0.35f + energy_ * 0.25f), plot.getCentreX(), top,
                                   mint.withAlpha (0.05f), plot.getCentreX(), bottom, false);
        g.setGradientFill (grad);
        g.fillPath (fill);

        g.setColour (col.withAlpha (0.9f));
        g.strokePath (p, juce::PathStrokeType (2.4f));
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.strokePath (p, juce::PathStrokeType (1.0f));

        // Moving scan bead along the curve
        const float t = 0.5f + 0.5f * std::sin (phase_);
        const float sx = ax + t * (endX - ax);
        float sy = bottom;
        if (sx <= peakX) sy = bottom + (top - bottom) * ((sx - ax) / juce::jmax (1.0f, peakX - ax));
        else if (sx <= susX) sy = top + (susY - top) * ((sx - peakX) / juce::jmax (1.0f, susX - peakX));
        else if (sx <= relX) sy = susY;
        else sy = susY + (bottom - susY) * ((sx - relX) / juce::jmax (1.0f, endX - relX));
        g.setColour (mint.withAlpha (0.85f));
        g.fillEllipse (sx - 4.0f, sy - 4.0f, 8.0f, 8.0f);

        auto dot = [&] (float x, float y) {
            g.setColour (juce::Colours::white);
            g.fillEllipse (x - 4.5f, y - 4.5f, 9.0f, 9.0f);
            g.setColour (col);
            g.drawEllipse (x - 4.5f, y - 4.5f, 9.0f, 9.0f, 1.2f);
        };
        dot (peakX, top);
        dot (susX, susY);
        dot (relX, susY);

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("ENVELOPE", plot.getX(), plot.getY() - 4.0f, 70.0f, 12.0f, juce::Justification::centredLeft);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        auto plot = getLocalBounds().toFloat().reduced (12.0f, 16.0f);
        const float nx = juce::jlimit (0.0f, 1.0f, (e.position.x - plot.getX()) / plot.getWidth());
        const float ny = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - plot.getY()) / plot.getHeight());
        if (nx < 0.25f) attack_ = 0.001f + nx * 4.0f;
        else if (nx < 0.5f) decay_ = 0.001f + (nx - 0.25f) * 4.0f;
        else if (nx < 0.75f) sustain_ = ny;
        else release_ = 0.001f + (nx - 0.75f) * 8.0f;
        if (onChanged) onChanged (attack_, decay_, sustain_, release_);
        repaint();
    }

private:
    void timerCallback() override
    {
        phase_ += 0.08f + energy_ * 0.06f;
        repaint();
    }

    ScorionLookAndFeel* laf_ = nullptr;
    float attack_ = 0.01f, decay_ = 0.25f, sustain_ = 0.7f, release_ = 0.35f;
    float energy_ = 0.0f, phase_ = 0.0f;
};
