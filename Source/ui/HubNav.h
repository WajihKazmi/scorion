#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include <cmath>
#include <functional>
#include <vector>

/** Left rail navigation for the sound-design hub. */
class HubNav : public juce::Component
{
public:
    enum class Page { Perform, Design, Vocals, Samples, Library, Features, Theme };

    HubNav()
    {
        items_ = {
            { Page::Perform, "Perform", "PLAY" },
            { Page::Design, "Design", "DSP" },
            { Page::Vocals, "Vocals", "VOX" },
            { Page::Samples, "Samples", "SMP" },
            { Page::Library, "Library", "LIB" },
            { Page::Features, "Features", "100+" },
            { Page::Theme, "Theme", "UI" },
        };
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    Page getPage() const noexcept { return page_; }
    std::function<void (Page)> onPageChanged;

    void setPage (Page p)
    {
        if (page_ == p) return;
        page_ = p;
        anim_ = 0.0f;
        repaint();
        if (onPageChanged) onPageChanged (page_);
    }

    void tick (float dt)
    {
        anim_ = juce::jmin (1.0f, anim_ + dt * 4.5f); // ~220ms ease toward settled
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawPanel (g, b, 20.0f);

        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (juce::FontOptions (10.0f).withStyle ("Medium"));
        g.drawText ("HUB", b.removeFromTop (36.0f).reduced (14.0f, 12.0f),
                    juce::Justification::centredLeft);

        const float ease = 1.0f - std::pow (1.0f - anim_, 3.0f);
        const int n = (int) items_.size();
        const float rowH = juce::jmax (44.0f, (b.getHeight() - 12.0f) / (float) n);

        for (int i = 0; i < n; ++i)
        {
            auto row = b.removeFromTop (rowH).reduced (10.0f, 4.0f);
            const bool on = items_[(size_t) i].page == page_;
            if (on)
            {
                auto fill = row;
                if (laf_ != nullptr)
                {
                    juce::ColourGradient grad (laf_->accent().withAlpha (0.22f + 0.1f * ease),
                                               fill.getX(), fill.getY(),
                                               laf_->accentBlue().withAlpha (0.08f),
                                               fill.getRight(), fill.getBottom(), false);
                    g.setGradientFill (grad);
                }
                else g.setColour (juce::Colours::cyan.withAlpha (0.2f));
                g.fillRoundedRectangle (fill, 14.0f);
                g.setColour ((laf_ != nullptr ? laf_->accent() : juce::Colours::cyan).withAlpha (0.55f));
                g.fillRoundedRectangle (fill.getX(), fill.getY() + 8.0f, 3.0f, fill.getHeight() - 16.0f, 1.5f);
            }
            else if (hover_ == i)
            {
                g.setColour (juce::Colours::white.withAlpha (0.04f));
                g.fillRoundedRectangle (row, 14.0f);
            }

            g.setColour (on ? (laf_ != nullptr ? laf_->accent() : juce::Colours::cyan)
                            : (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey));
            g.setFont (juce::FontOptions (11.0f).withStyle ("Bold"));
            g.drawText (items_[(size_t) i].badge, row.removeFromLeft (40.0f), juce::Justification::centred);

            g.setColour (on ? (laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white)
                            : (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey));
            g.setFont (juce::FontOptions (13.0f).withStyle (on ? "Medium" : "Regular"));
            g.drawText (items_[(size_t) i].label, row, juce::Justification::centredLeft);
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int h = hit (e.getPosition());
        if (h != hover_) { hover_ = h; repaint(); }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hover_ = -1;
        repaint();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        const int h = hit (e.getPosition());
        if (h >= 0)
            setPage (items_[(size_t) h].page);
    }

private:
    struct Item { Page page; juce::String label; juce::String badge; };
    std::vector<Item> items_;
    Page page_ = Page::Perform;
    ScorionLookAndFeel* laf_ = nullptr;
    int hover_ = -1;
    float anim_ = 1.0f;

    int hit (juce::Point<int> p) const
    {
        auto b = getLocalBounds().toFloat();
        b.removeFromTop (36.0f);
        const int n = (int) items_.size();
        const float rowH = juce::jmax (44.0f, (b.getHeight() - 12.0f) / (float) n);
        if (! b.contains (p.toFloat())) return -1;
        const int i = (int) ((p.y - b.getY()) / rowH);
        return juce::isPositiveAndBelow (i, n) ? i : -1;
    }
};
