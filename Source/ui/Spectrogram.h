#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <array>
#include <cmath>
#include <deque>

/** Scrolling spectrogram with smoothed columns (less sparkle/glitch). */
class Spectrogram : public juce::Component
{
public:
    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }

    void pushBands (const std::array<float, 64>& bands)
    {
        // Temporal + vertical smoothing into last column
        Column target {};
        for (int i = 0; i < kRows; ++i)
        {
            const int a0 = i * 64 / kRows;
            const int a1 = (i + 1) * 64 / kRows;
            float e = 0.0f;
            for (int j = a0; j < a1; ++j) e += bands[(size_t) j];
            target[(size_t) i] = e / (float) juce::jmax (1, a1 - a0);
        }

        if (! history_.empty())
        {
            auto& prev = history_.back();
            for (int i = 0; i < kRows; ++i)
                target[(size_t) i] = prev[(size_t) i] * 0.72f + target[(size_t) i] * 0.28f;
        }

        // Spatial blur
        Column blur = target;
        for (int i = 1; i < kRows - 1; ++i)
            blur[(size_t) i] = 0.2f * target[(size_t) (i - 1)] + 0.6f * target[(size_t) i]
                             + 0.2f * target[(size_t) (i + 1)];

        // Only append every other push — slower scroll, calmer motion
        if ((++pushCount_ % 2) == 0)
        {
            history_.push_back (blur);
            while ((int) history_.size() > maxCols_)
                history_.pop_front();
        }
        else if (! history_.empty())
            history_.back() = blur;

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
            for (int r = 0; r < kRows; ++r)
            {
                const float v = col[(size_t) r];
                if (v < 0.03f) continue;
                const float y = plot.getBottom() - ((float) (r + 1) / (float) kRows) * plot.getHeight();
                const float h = plot.getHeight() / (float) kRows;
                auto c = (laf_ != nullptr ? laf_->ember() : juce::Colours::white)
                             .interpolatedWith (laf_ != nullptr ? laf_->mint() : juce::Colours::cyan,
                                                (float) r / (float) kRows);
                g.setColour (c.withAlpha (0.10f + v * 0.65f));
                g.fillRect (x, y, cw + 0.8f, h + 0.6f);
            }
            ++ci;
        }

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("SPECTROGRAM", plot.getX(), plot.getY() - 2.0f, 90.0f, 12.0f, juce::Justification::centredLeft);
        juce::ignoreUnused (energy_);
    }

private:
    static constexpr int kRows = 32;
    using Column = std::array<float, kRows>;
    ScorionLookAndFeel* laf_ = nullptr;
    std::deque<Column> history_;
    int maxCols_ = 72;
    int pushCount_ = 0;
    float energy_ = 0.0f;
};
