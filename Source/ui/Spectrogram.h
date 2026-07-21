#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <array>
#include <cmath>
#include <deque>

/** Scrolling spectrogram heat strip driven by probe bands. */
class Spectrogram : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }

    void pushBands (const std::array<float, 64>& bands)
    {
        Column col {};
        for (int i = 0; i < 64; ++i)
            col[(size_t) i] = bands[(size_t) i];
        history_.push_back (col);
        while ((int) history_.size() > maxCols_)
            history_.pop_front();
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawInsetWell (g, b, 12.0f);

        auto plot = b.reduced (8.0f, 12.0f);
        const int cols = (int) history_.size();
        if (cols < 2) return;

        const float cw = plot.getWidth() / (float) juce::jmax (1, maxCols_);
        int ci = 0;
        for (const auto& col : history_)
        {
            const float x = plot.getX() + (float) ci * cw;
            for (int r = 0; r < 64; ++r)
            {
                const float v = col[(size_t) r];
                if (v < 0.02f) continue;
                const float y = plot.getBottom() - ((float) (r + 1) / 64.0f) * plot.getHeight();
                const float h = plot.getHeight() / 64.0f;
                auto c = (laf_ != nullptr ? laf_->ember() : juce::Colour (0xffC1122F))
                             .interpolatedWith (laf_ != nullptr ? laf_->violet() : juce::Colour (0xff5E3B76),
                                                (float) r / 64.0f);
                g.setColour (c.withAlpha (0.15f + v * 0.75f));
                g.fillRect (x, y, cw + 0.5f, h + 0.5f);
            }
            ++ci;
        }

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("SPECTROGRAM", plot.getX(), plot.getY() - 2.0f, 90.0f, 12.0f, juce::Justification::centredLeft);
    }

private:
    using Column = std::array<float, 64>;
    ScorionLookAndFeel* laf_ = nullptr;
    std::deque<Column> history_;
    int maxCols_ = 96;
    float energy_ = 0.0f;
};
