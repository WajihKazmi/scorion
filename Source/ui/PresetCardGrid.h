#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "presets/PresetManager.h"
#include <memory>
#include <functional>
#include <vector>

/** Portal-style preset library: image cards + category chips + search. */
class PresetCardGrid : public juce::Component,
                       private juce::Timer
{
public:
    explicit PresetCardGrid (PresetManager& pm) : presets_ (pm)
    {
        search.setTextToShowWhenEmpty ("Filter library...", juce::Colours::grey);
        search.onTextChange = [this] { rebuild(); };
        addAndMakeVisible (search);

        catBox.addItem ("All", 1);
        catBox.onChange = [this] { rebuild(); };
        addAndMakeVisible (catBox);

        viewport.setViewedComponent (&grid, false);
        viewport.setScrollBarsShown (true, false);
        addAndMakeVisible (viewport);

        startTimerHz (24);
        rebuildCategories();
        rebuild();
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf != nullptr)
        {
            search.setColour (juce::TextEditor::backgroundColourId, laf->surfaceHi());
            search.setColour (juce::TextEditor::textColourId, laf->textPrimary());
            search.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            search.setTextToShowWhenEmpty ("Filter library...", laf->textSecondary());
            catBox.setColour (juce::ComboBox::backgroundColourId, laf->surfaceHi());
            catBox.setColour (juce::ComboBox::textColourId, laf->textPrimary());
        }
        repaint();
    }

    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setSelectedIndex (int idx) { selected_ = idx; grid.repaint(); }
    std::function<void (int index)> onSelect;

    void refreshFromManager()
    {
        rebuildCategories();
        rebuild();
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText ("LIBRARY", 8, 4, 80, 12, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->ember() : juce::Colours::orange);
        g.setFont (laf_ != nullptr ? laf_->uiFont (11.0f) : juce::FontOptions (11.0f));
        g.drawText (juce::String ((int) visible_.size()) + " presets",
                    getWidth() - 90, 4, 82, 12, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        a.removeFromTop (18);
        auto top = a.removeFromTop (30);
        search.setBounds (top.removeFromLeft ((int) (top.getWidth() * 0.58f)).reduced (2));
        catBox.setBounds (top.reduced (2));
        a.removeFromTop (8);
        viewport.setBounds (a);
        layoutGrid();
    }

private:
    struct CardButton : public juce::Component
    {
        PresetCardGrid& owner;
        int index = -1;
        juce::String name, category, engine;

        explicit CardButton (PresetCardGrid& o) : owner (o) {}

        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced (3.0f);
            const bool sel = owner.selected_ == index;
            auto* laf = owner.laf_;
            const auto base = laf != nullptr ? laf->surfaceHi() : juce::Colour (0xff16101c);
            const auto ember = laf != nullptr ? laf->ember() : juce::Colour (0xffff1e48);
            const auto mint = laf != nullptr ? laf->mint() : juce::Colour (0xffb84dff);

            g.setColour (base);
            g.fillRoundedRectangle (b, 12.0f);
            if (sel)
            {
                g.setColour (ember.withAlpha (0.85f));
                g.drawRoundedRectangle (b.reduced (0.5f), 12.0f, 1.8f);
            }
            else
            {
                g.setColour ((laf != nullptr ? laf->violet() : juce::Colours::purple)
                                 .withAlpha (0.25f + owner.energy_ * 0.15f));
                g.drawRoundedRectangle (b.reduced (0.5f), 12.0f, 1.0f);
            }

            // Procedural "cover art"
            auto art = b.removeFromTop (b.getHeight() * 0.58f).reduced (6.0f, 6.0f);
            paintCover (g, art, name, category, ember, mint);

            g.setColour (laf != nullptr ? laf->textPrimary() : juce::Colours::white);
            g.setFont (laf != nullptr ? laf->uiFont (12.0f, true) : juce::FontOptions (12.0f));
            g.drawText (name, b.getX() + 8.0f, b.getY() + 2.0f, b.getWidth() - 16.0f, 16.0f,
                        juce::Justification::centredLeft, true);
            g.setColour (laf != nullptr ? laf->textSecondary() : juce::Colours::grey);
            g.setFont (laf != nullptr ? laf->uiFont (10.0f) : juce::FontOptions (10.0f));
            g.drawText (category, b.getX() + 8.0f, b.getY() + 18.0f, b.getWidth() - 16.0f, 14.0f,
                        juce::Justification::centredLeft, true);
        }

        void paintCover (juce::Graphics& g, juce::Rectangle<float> art,
                         const juce::String& n, const juce::String& cat,
                         juce::Colour a, juce::Colour bCol)
        {
            const auto h = (uint32_t) (n.hashCode() ^ cat.hashCode());
            const float hueShift = (float) (h % 1000) / 1000.0f;
            auto c1 = a.interpolatedWith (bCol, hueShift * 0.65f);
            auto c2 = bCol.darker (0.35f + (float) ((h >> 8) % 40) / 100.0f);

            juce::ColourGradient grad (c1.withAlpha (0.9f), art.getTopLeft(),
                                       c2.withAlpha (0.55f), art.getBottomRight(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (art, 9.0f);

            // Abstract shapes from hash
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            const float cx = art.getCentreX() + ((int) (h % 21) - 10) * 1.5f;
            const float cy = art.getCentreY() + ((int) ((h >> 4) % 17) - 8) * 1.2f;
            const float r = art.getHeight() * (0.22f + (float) (h % 30) / 120.0f);
            g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

            g.setColour (c1.brighter (0.3f).withAlpha (0.35f + owner.energy_ * 0.25f));
            juce::Path wave;
            const int pts = 24;
            for (int i = 0; i < pts; ++i)
            {
                const float t = (float) i / (float) (pts - 1);
                const float x = art.getX() + t * art.getWidth();
                const float y = art.getCentreY()
                               + std::sin (t * 6.283f * (1.5f + (h % 3)) + owner.phase_ + (float) (h % 7))
                                     * art.getHeight() * 0.18f;
                if (i == 0) wave.startNewSubPath (x, y);
                else wave.lineTo (x, y);
            }
            g.strokePath (wave, juce::PathStrokeType (1.6f));

            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRoundedRectangle (art.getX(), art.getBottom() - 18.0f, art.getWidth(), 18.0f, 0.0f);
            g.setColour (juce::Colours::white.withAlpha (0.75f));
            g.setFont (juce::FontOptions (9.5f));
            g.drawText (engine, art.reduced (6.0f, 2.0f).removeFromBottom (16.0f),
                        juce::Justification::centredLeft, true);
        }

        void mouseDown (const juce::MouseEvent&) override
        {
            owner.selected_ = index;
            if (owner.onSelect) owner.onSelect (index);
            owner.grid.repaint();
        }
    };

    struct GridComp : public juce::Component
    {
        PresetCardGrid& owner;
        std::vector<std::unique_ptr<CardButton>> cards;
        explicit GridComp (PresetCardGrid& o) : owner (o) {}
        void resized() override { owner.layoutGrid(); }
    };

    void rebuildCategories()
    {
        juce::StringArray cats;
        cats.add ("All");
        for (const auto& p : presets_.presets())
            if (! cats.contains (p.category))
                cats.add (p.category);
        cats.sort (true);
        // Keep All first
        cats.removeString ("All");
        cats.insert (0, "All");
        catBox.clear (juce::dontSendNotification);
        for (int i = 0; i < cats.size(); ++i)
            catBox.addItem (cats[i], i + 1);
        catBox.setSelectedId (1, juce::dontSendNotification);
    }

    void rebuild()
    {
        const auto cat = catBox.getText();
        const auto q = search.getText().trim();
        visible_ = presets_.filteredIndices (cat.isEmpty() ? "All" : cat, q);
        grid.cards.clear();
        grid.removeAllChildren();
        for (int vi : visible_)
        {
            if (! juce::isPositiveAndBelow (vi, (int) presets_.presets().size())) continue;
            const auto& p = presets_.presets()[(size_t) vi];
            auto card = std::make_unique<CardButton> (*this);
            card->index = vi;
            card->name = p.name;
            card->category = p.category;
            card->engine = p.engine;
            grid.addAndMakeVisible (*card);
            grid.cards.push_back (std::move (card));
        }
        layoutGrid();
        repaint();
    }

    void layoutGrid()
    {
        const int cols = juce::jmax (1, viewport.getWidth() / 168);
        const int cardW = juce::jmax (140, (viewport.getWidth() - 8) / cols);
        const int cardH = 148;
        int i = 0;
        for (auto& c : grid.cards)
        {
            const int col = i % cols;
            const int row = i / cols;
            c->setBounds (col * cardW + 2, row * cardH + 2, cardW - 4, cardH - 4);
            ++i;
        }
        const int rows = (int) ((grid.cards.size() + cols - 1) / juce::jmax (1, cols));
        grid.setSize (viewport.getWidth(), juce::jmax (viewport.getHeight(), rows * cardH + 8));
    }

    void timerCallback() override
    {
        phase_ += 0.06f + energy_ * 0.05f;
        if (! grid.cards.empty())
            grid.repaint();
    }

    PresetManager& presets_;
    ScorionLookAndFeel* laf_ = nullptr;
    juce::TextEditor search;
    juce::ComboBox catBox;
    juce::Viewport viewport;
    GridComp grid { *this };
    std::vector<int> visible_;
    int selected_ = -1;
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
