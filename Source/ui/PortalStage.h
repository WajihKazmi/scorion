#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <array>
#include <cmath>

/** Portal-style circular XY energy stage — always in motion. */
class PortalStage : public juce::Component,
                    private juce::Timer
{
public:
    PortalStage() { startTimerHz (40); }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setSnapshot (const std::array<float, 256>& data)
    {
        for (int i = 0; i < kSpokes; ++i)
        {
            const int a = i * 256 / kSpokes;
            const int b = (i + 1) * 256 / kSpokes;
            float e = 0.0f;
            for (int j = a; j < b; ++j) e += data[(size_t) j] * data[(size_t) j];
            e = std::sqrt (e / (float) juce::jmax (1, b - a));
            spokes_[(size_t) i] += 0.3f * (juce::jlimit (0.0f, 1.0f, e * 7.0f) - spokes_[(size_t) i]);
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, bounds, 16.0f);

        const auto c = bounds.getCentre();
        const float R = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.42f;
        const auto crimson = laf_ != nullptr ? laf_->ember() : juce::Colour (0xffff1e48);
        const auto purple = laf_ != nullptr ? laf_->mint() : juce::Colour (0xffb84dff);
        const auto violet = laf_ != nullptr ? laf_->violet() : juce::Colour (0xff6a22c8);

        // Soft radial void
        {
            juce::ColourGradient rg (crimson.withAlpha (0.08f + energy_ * 0.18f), c.x, c.y,
                                     juce::Colours::transparentBlack, c.x + R, c.y, true);
            g.setGradientFill (rg);
            g.fillEllipse (c.x - R, c.y - R, R * 2.0f, R * 2.0f);
        }

        // Live particles reacting to audio
        for (int i = 0; i < kParticles; ++i)
        {
            const float life = particles_[(size_t) i].life;
            if (life <= 0.0f) continue;
            const float px = c.x + particles_[(size_t) i].x * R;
            const float py = c.y + particles_[(size_t) i].y * R;
            const float pr = 1.5f + life * 3.5f * (0.4f + energy_);
            g.setColour ((i % 2 == 0 ? crimson : purple).withAlpha (life * 0.7f));
            g.fillEllipse (px - pr, py - pr, pr * 2.0f, pr * 2.0f);
        }

        // Orbit rings
        for (int ring = 0; ring < 4; ++ring)
        {
            const float t = (float) ring / 3.0f;
            const float r = R * (0.35f + t * 0.65f);
            const float spin = phase_ * (0.4f + t * 0.5f) * (ring % 2 == 0 ? 1.0f : -1.0f);
            g.setColour (violet.withAlpha (0.12f + energy_ * 0.15f * (1.0f - t)));
            g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 1.2f);

            const float mx = c.x + std::cos (spin + t * 2.0f) * r;
            const float my = c.y + std::sin (spin + t * 2.0f) * r;
            g.setColour ((ring % 2 == 0 ? crimson : purple).withAlpha (0.55f + energy_ * 0.35f));
            g.fillEllipse (mx - 3.0f, my - 3.0f, 6.0f, 6.0f);
        }

        // Audio spokes (XY ring)
        juce::Path poly;
        for (int i = 0; i < kSpokes; ++i)
        {
            const float ang = phase_ * 0.35f + juce::MathConstants<float>::twoPi * (float) i / (float) kSpokes;
            const float mag = R * (0.42f + spokes_[(size_t) i] * 0.5f * (0.35f + energy_));
            const float x = c.x + std::cos (ang) * mag;
            const float y = c.y + std::sin (ang) * mag;
            if (i == 0) poly.startNewSubPath (x, y);
            else poly.lineTo (x, y);
        }
        poly.closeSubPath();
        g.setColour (crimson.withAlpha (0.18f + energy_ * 0.25f));
        g.fillPath (poly);
        g.setColour (crimson.withAlpha (0.75f));
        g.strokePath (poly, juce::PathStrokeType (2.0f));
        g.setColour (purple.withAlpha (0.55f));
        g.strokePath (poly, juce::PathStrokeType (1.0f));

        // Core
        const float core = 10.0f + energy_ * 14.0f;
        juce::ColourGradient cg (juce::Colours::white.withAlpha (0.9f), c.x, c.y - 4.0f,
                                 crimson, c.x, c.y + core, true);
        g.setGradientFill (cg);
        g.fillEllipse (c.x - core, c.y - core, core * 2.0f, core * 2.0f);

        g.setColour ((laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey).withAlpha (0.7f));
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("PORTAL STAGE", bounds.reduced (14.0f, 10.0f), juce::Justification::topLeft);
    }

private:
    struct Particle { float x = 0, y = 0, vx = 0, vy = 0, life = 0; };

    void timerCallback() override
    {
        phase_ += 0.045f + energy_ * 0.08f;
        // Spawn / update particles
        if (energy_ > 0.08f && (int) (phase_ * 10.0f) % 3 == 0)
        {
            for (auto& p : particles_)
            {
                if (p.life <= 0.0f)
                {
                    const float ang = phase_ * 1.7f + (float) (&p - particles_.data()) * 0.4f;
                    p.x = std::cos (ang) * 0.15f;
                    p.y = std::sin (ang) * 0.15f;
                    p.vx = std::cos (ang) * (0.01f + energy_ * 0.02f);
                    p.vy = std::sin (ang) * (0.01f + energy_ * 0.02f);
                    p.life = 0.6f + energy_ * 0.4f;
                    break;
                }
            }
        }
        for (auto& p : particles_)
        {
            if (p.life <= 0.0f) continue;
            p.x += p.vx;
            p.y += p.vy;
            p.life -= 0.02f;
        }
        repaint();
    }

    static constexpr int kSpokes = 48;
    static constexpr int kParticles = 24;
    ScorionLookAndFeel* laf_ = nullptr;
    std::array<float, kSpokes> spokes_ {};
    std::array<Particle, kParticles> particles_ {};
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
