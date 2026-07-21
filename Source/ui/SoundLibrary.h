#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "Motion.h"
#include "presets/PresetManager.h"
#include <functional>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

/** Sound library with clearer typography, sort, and denser card UX. */
class SoundLibrary : public juce::Component,
                     private juce::Timer
{
public:
    explicit SoundLibrary (PresetManager& pm) : presets_ (pm)
    {
        search.setTextToShowWhenEmpty ("Search… dark horror choir", juce::Colours::grey);
        search.setFont (juce::FontOptions (14.0f));
        search.onTextChange = [this] { rebuild(); };
        addAndMakeVisible (search);

        favOnly.setButtonText ("♥ Fav");
        favOnly.setClickingTogglesState (true);
        favOnly.onClick = [this] { rebuild(); };
        addAndMakeVisible (favOnly);

        sortBox.addItemList ({ "Name", "Category", "Energy ↓", "Mood" }, 1);
        sortBox.setSelectedId (1, juce::dontSendNotification);
        sortBox.onChange = [this] { rebuild(); };
        addAndMakeVisible (sortBox);

        for (const auto& c : collections_)
        {
            auto* b = chips.add (new juce::TextButton (c));
            b->setClickingTogglesState (true);
            b->setRadioGroupId (9101);
            b->onClick = [this, c] {
                activeCollection_ = c;
                rebuild();
            };
            addAndMakeVisible (b);
        }
        if (chips.size() > 0)
            chips[0]->setToggleState (true, juce::dontSendNotification);

        chipViewport.setViewedComponent (&chipRow, false);
        chipViewport.setScrollBarsShown (false, true);
        addAndMakeVisible (chipViewport);
        for (auto* b : chips)
            chipRow.addAndMakeVisible (b);

        viewport.setViewedComponent (&grid, false);
        viewport.setScrollBarsShown (true, false);
        addAndMakeVisible (viewport);

        hint.setText ("Click card to load + audition  ·  Heart to favorite", juce::dontSendNotification);
        hint.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (hint);

        startTimerHz (24);
        rebuild();
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        if (laf != nullptr)
        {
            search.setFont (laf->uiFont (14.0f, true));
            search.setColour (juce::TextEditor::backgroundColourId, laf->card());
            search.setColour (juce::TextEditor::textColourId, laf->textPrimary());
            search.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            search.setTextToShowWhenEmpty ("Search… dark horror choir", laf->textSecondary());
            hint.setFont (laf->uiFont (12.0f));
            hint.setColour (juce::Label::textColourId, laf->textSecondary());
            sortBox.setColour (juce::ComboBox::textColourId, laf->textPrimary());
        }
        repaint();
    }

    void setExternalQuery (const juce::String& q)
    {
        if (search.getText() != q)
        {
            search.setText (q, juce::dontSendNotification);
            rebuild();
        }
    }

    void setEnergy (float e) { energy_ = juce::jlimit (0.0f, 1.0f, e); }
    void setSelectedIndex (int idx) { selected_ = idx; grid.repaint(); }
    void refreshFromManager() { rebuild(); }

    std::function<void (int index)> onSelect;
    std::function<void (int index)> onFavorite;
    std::function<void (int index)> onPreview;

    void paint (juce::Graphics& g) override
    {
        g.setColour (laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white);
        g.setFont (laf_ != nullptr ? laf_->titleFont (15.0f) : juce::FontOptions (15.0f));
        g.drawText ("SOUND LIBRARY", 8, 4, 180, 18, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->mint() : juce::Colours::cyan);
        g.setFont (laf_ != nullptr ? laf_->uiFont (13.0f, true) : juce::FontOptions (13.0f));
        g.drawText (juce::String ((int) visible_.size()) + " sounds",
                    getWidth() - 110, 4, 100, 18, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        a.removeFromTop (22);
        auto top = a.removeFromTop (32);
        favOnly.setBounds (top.removeFromRight (64).reduced (2));
        sortBox.setBounds (top.removeFromRight (100).reduced (2));
        search.setBounds (top.reduced (2));
        a.removeFromTop (8);
        chipViewport.setBounds (a.removeFromTop (34));
        {
            int x = 0;
            for (auto* b : chips)
            {
                const int w = juce::jmax (72, b->getButtonText().length() * 9 + 20);
                b->setBounds (x, 2, w, 30);
                x += w + 4;
            }
            chipRow.setSize (x + 8, 34);
        }
        a.removeFromTop (6);
        hint.setBounds (a.removeFromTop (18));
        a.removeFromTop (4);
        viewport.setBounds (a);
        layoutGrid();
    }

private:
    struct CardButton : public juce::Component
    {
        SoundLibrary& owner;
        int index = -1;
        PresetInfo info;
        float hover = 0.0f;

        explicit CardButton (SoundLibrary& o) : owner (o)
        {
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
        }

        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced (3.0f - hover * 1.5f);
            auto* laf = owner.laf_;
            const bool sel = owner.selected_ == index;
            if (laf != nullptr)
                laf->drawCard (g, b, sel, hover > 0.2f, 12.0f);

            auto art = b.removeFromTop (b.getHeight() * 0.48f).reduced (8.0f, 8.0f);
            paintCover (g, art);

            const bool fav = owner.presets_.isFavorite (info.name);
            g.setColour (fav ? (laf != nullptr ? laf->mint() : juce::Colours::cyan)
                             : (laf != nullptr ? laf->textSecondary() : juce::Colours::grey));
            g.setFont (juce::FontOptions (16.0f));
            g.drawText (fav ? juce::CharPointer_UTF8 ("\xe2\x99\xa5") : juce::CharPointer_UTF8 ("\xe2\x99\xa1"),
                        (int) b.getRight() - 26, (int) b.getY() + 4, 22, 18, juce::Justification::centred);

            g.setColour (laf != nullptr ? laf->textPrimary() : juce::Colours::white);
            g.setFont (laf != nullptr ? laf->uiFont (16.0f, true) : juce::FontOptions (16.0f));
            g.drawText (info.name, (int) b.getX() + 10, (int) b.getY() + 2, (int) b.getWidth() - 36, 22,
                        juce::Justification::centredLeft, true);

            auto chip = juce::Rectangle<float> (b.getX() + 10.0f, b.getY() + 28.0f, 86.0f, 18.0f);
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.28f));
            g.fillRoundedRectangle (chip, 8.0f);
            g.setColour (laf != nullptr ? laf->textPrimary() : juce::Colours::white);
            g.setFont (laf != nullptr ? laf->uiFont (12.0f, true) : juce::FontOptions (12.0f));
            g.drawText (info.mood, chip, juce::Justification::centred, true);

            g.setColour (laf != nullptr ? laf->textSecondary() : juce::Colours::grey);
            g.setFont (laf != nullptr ? laf->uiFont (12.5f, true) : juce::FontOptions (12.5f));
            const auto kind = info.kind == LibraryItemKind::Wavetable ? "Wavetable"
                            : info.kind == LibraryItemKind::Sample ? "Sample" : info.category;
            g.drawText (kind, (int) chip.getRight() + 8, (int) chip.getY(), 110, 18,
                        juce::Justification::centredLeft, true);

            // PLAY badge on hover
            if (hover > 0.4f)
            {
                auto badge = juce::Rectangle<float> (art.getRight() - 52.0f, art.getBottom() - 22.0f, 46.0f, 18.0f);
                g.setColour ((laf != nullptr ? laf->ember() : juce::Colours::white).withAlpha (0.9f));
                g.fillRoundedRectangle (badge, 6.0f);
                g.setColour (laf != nullptr && laf->isLightTheme() ? juce::Colours::white : juce::Colours::black);
                g.setFont (laf != nullptr ? laf->uiFont (10.0f, true) : juce::FontOptions (10.0f));
                g.drawText ("PLAY", badge, juce::Justification::centred);
            }

            auto wave = b.removeFromBottom (18.0f).reduced (10.0f, 3.0f);
            paintWave (g, wave);
        }

        void paintCover (juce::Graphics& g, juce::Rectangle<float> art)
        {
            auto* laf = owner.laf_;
            const auto h = (uint32_t) info.artworkSeed;
            const float t = (float) (h % 1000) / 1000.0f;
            auto c1 = (laf != nullptr ? laf->ember() : juce::Colours::white).interpolatedWith (
                laf != nullptr ? laf->mint() : juce::Colours::cyan, t * 0.7f);
            auto c2 = (laf != nullptr ? laf->surface() : juce::Colour (0xff111111));
            juce::ColourGradient grad (c1.withAlpha (0.75f), art.getTopLeft(),
                                       c2, art.getBottomRight(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (art, 10.0f);
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.drawRoundedRectangle (art.reduced (0.5f), 10.0f, 1.0f);

            juce::Path wave;
            for (int i = 0; i < 32; ++i)
            {
                const float u = (float) i / 31.0f;
                const float x = art.getX() + u * art.getWidth();
                const float y = art.getCentreY()
                    + std::sin (u * 6.28f * (1.2f + (h % 3)) + owner.phase_)
                      * art.getHeight() * 0.22f * (0.45f + info.energy);
                if (i == 0) wave.startNewSubPath (x, y);
                else wave.lineTo (x, y);
            }
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.7f));
            g.strokePath (wave, juce::PathStrokeType (2.0f));
        }

        void paintWave (juce::Graphics& g, juce::Rectangle<float> wave)
        {
            auto* laf = owner.laf_;
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.65f));
            juce::Path p;
            for (int i = 0; i < 40; ++i)
            {
                const float u = (float) i / 39.0f;
                const float seed = (float) ((info.artworkSeed >> (i % 8)) & 7) / 7.0f;
                const float hh = (0.2f + seed * 0.75f) * wave.getHeight();
                const float x = wave.getX() + u * wave.getWidth();
                const float y = wave.getBottom() - hh;
                if (i == 0) p.startNewSubPath (x, y);
                else p.lineTo (x, y);
            }
            g.strokePath (p, juce::PathStrokeType (1.4f));
        }

        void mouseEnter (const juce::MouseEvent&) override { hover = 1.0f; repaint(); }
        void mouseExit (const juce::MouseEvent&) override { hover = 0.0f; repaint(); }

        void mouseDown (const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu() || (e.x > getWidth() - 30 && e.y < 30))
            {
                if (owner.onFavorite) owner.onFavorite (index);
                repaint();
                return;
            }
            owner.selected_ = index;
            if (owner.onSelect) owner.onSelect (index);
            if (owner.onPreview) owner.onPreview (index);
            owner.grid.repaint();
        }
    };

    struct GridComp : public juce::Component
    {
        SoundLibrary& owner;
        std::vector<std::unique_ptr<CardButton>> cards;
        explicit GridComp (SoundLibrary& o) : owner (o) {}
        void resized() override { owner.layoutGrid(); }
    };

    struct ChipRow : public juce::Component {};

    void rebuild()
    {
        visible_ = presets_.filteredIndices (activeCollection_, search.getText(),
                                             favOnly.getToggleState());
        // Sort
        const int sortMode = sortBox.getSelectedId();
        std::sort (visible_.begin(), visible_.end(), [&] (int a, int b) {
            const auto& pa = presets_.presets()[(size_t) a];
            const auto& pb = presets_.presets()[(size_t) b];
            if (sortMode == 2) return pa.category.compareIgnoreCase (pb.category) < 0;
            if (sortMode == 3) return pa.energy > pb.energy;
            if (sortMode == 4) return pa.mood.compareIgnoreCase (pb.mood) < 0;
            return pa.name.compareIgnoreCase (pb.name) < 0;
        });

        grid.cards.clear();
        grid.removeAllChildren();
        for (int vi : visible_)
        {
            if (! juce::isPositiveAndBelow (vi, (int) presets_.presets().size())) continue;
            auto card = std::make_unique<CardButton> (*this);
            card->index = vi;
            card->info = presets_.presets()[(size_t) vi];
            grid.addAndMakeVisible (*card);
            grid.cards.push_back (std::move (card));
        }
        layoutGrid();
        repaint();
    }

    void layoutGrid()
    {
        const int cols = juce::jmax (1, viewport.getWidth() / 176);
        const int cardW = juce::jmax (160, (viewport.getWidth() - 8) / cols);
        const int cardH = 168;
        int i = 0;
        for (auto& c : grid.cards)
        {
            c->setBounds ((i % cols) * cardW + 2, (i / cols) * cardH + 2, cardW - 4, cardH - 4);
            ++i;
        }
        const int rows = (int) ((grid.cards.size() + cols - 1) / juce::jmax (1, cols));
        grid.setSize (viewport.getWidth(), juce::jmax (viewport.getHeight(), rows * cardH + 8));
    }

    void timerCallback() override
    {
        phase_ += 0.05f + energy_ * 0.04f;
        if (! grid.cards.empty())
            grid.repaint();
    }

    PresetManager& presets_;
    ScorionLookAndFeel* laf_ = nullptr;
    juce::TextEditor search;
    juce::TextButton favOnly;
    juce::ComboBox sortBox;
    juce::Label hint;
    juce::OwnedArray<juce::TextButton> chips;
    juce::Viewport chipViewport, viewport;
    ChipRow chipRow;
    GridComp grid { *this };
    std::vector<int> visible_;
    juce::String activeCollection_ { "All" };
    juce::StringArray collections_ { "All", "Favorites", "Bass", "Pads", "Leads", "Keys",
                                     "Vocals", "Textures", "Cinematic", "Experimental", "FX" };
    int selected_ = -1;
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
