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

/** Sound library — wrap categories, clearer cards, less chrome congestion. */
class SoundLibrary : public juce::Component,
                     private juce::Timer
{
public:
    explicit SoundLibrary (PresetManager& pm) : presets_ (pm)
    {
        search.setTextToShowWhenEmpty ("Search library…", juce::Colours::grey);
        search.setFont (juce::FontOptions (14.0f));
        search.onTextChange = [this] { rebuild(); };
        addAndMakeVisible (search);

        sortBox.addItemList ({ "Name", "Category", "Energy ↓", "Mood" }, 1);
        sortBox.setSelectedId (1, juce::dontSendNotification);
        sortBox.onChange = [this] { rebuild(); };
        addAndMakeVisible (sortBox);

        for (const auto& c : collections_)
        {
            auto* b = chips.add (new juce::TextButton (c));
            b->setClickingTogglesState (true);
            b->setRadioGroupId (9101);
            b->setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
            b->onClick = [this, c] {
                activeCollection_ = c;
                rebuild();
            };
            chipWrap.addAndMakeVisible (b);
        }
        if (chips.size() > 0)
            chips[0]->setToggleState (true, juce::dontSendNotification);
        addAndMakeVisible (chipWrap);

        viewport.setViewedComponent (&grid, false);
        viewport.setScrollBarsShown (true, false);
        addAndMakeVisible (viewport);

        hint.setText ("Click to load + audition  ·  ♥ favorite", juce::dontSendNotification);
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
            search.setTextToShowWhenEmpty ("Search library…", laf->textSecondary());
            hint.setFont (laf->uiFont (11.5f));
            hint.setColour (juce::Label::textColourId, laf->textSecondary());
            sortBox.setColour (juce::ComboBox::textColourId, laf->textPrimary());
            for (auto* b : chips)
            {
                b->setColour (juce::TextButton::textColourOffId, laf->textPrimary());
                b->setColour (juce::TextButton::textColourOnId, laf->isLightTheme()
                               ? juce::Colours::white : juce::Colours::black);
            }
        }
        repaint();
        resized();
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
        g.setFont (laf_ != nullptr ? laf_->titleFont (14.0f) : juce::FontOptions (14.0f));
        g.drawText ("SOUND LIBRARY", 8, 2, 160, 16, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->mint() : juce::Colours::cyan);
        g.setFont (laf_ != nullptr ? laf_->uiFont (12.0f, true) : juce::FontOptions (12.0f));
        g.drawText (juce::String ((int) visible_.size()) + " sounds",
                    getWidth() - 96, 2, 88, 16, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        a.removeFromTop (18);

        auto searchRow = a.removeFromTop (30);
        sortBox.setBounds (searchRow.removeFromRight (96).reduced (2));
        searchRow.removeFromRight (4);
        search.setBounds (searchRow.reduced (0, 1));

        a.removeFromTop (6);
        const int chipH = chipWrap.layout (a.getWidth());
        chipWrap.setBounds (a.removeFromTop (chipH));

        a.removeFromTop (4);
        hint.setBounds (a.removeFromTop (16));
        a.removeFromTop (2);
        viewport.setBounds (a);
        layoutGrid();
    }

private:
    /** Wraps category chips so every label stays fully visible (no horizontal clip). */
    struct ChipWrap : public juce::Component
    {
        int layout (int width)
        {
            const int gap = 5;
            const int rowH = 28;
            int x = 0, y = 0, rows = 1;
            for (auto* c : getChildren())
            {
                auto* b = dynamic_cast<juce::Button*> (c);
                if (b == nullptr) continue;
                const int w = juce::jmax (52, b->getButtonText().length() * 8 + 18);
                if (x > 0 && x + w > width)
                {
                    x = 0;
                    y += rowH + gap;
                    ++rows;
                }
                b->setBounds (x, y, w, rowH);
                x += w + gap;
            }
            return y + rowH;
        }
    };

    struct HeartButton : public juce::Button
    {
        HeartButton() : juce::Button ("fav") {}
        juce::Colour onCol { 0xff00f0ff }, offCol { 0xff8a8a8a };
        void paintButton (juce::Graphics& g, bool over, bool down) override
        {
            g.setColour ((getButtonText().contains (juce::CharPointer_UTF8 ("\xe2\x99\xa5")) ? onCol : offCol)
                             .withAlpha (over || down ? 1.0f : 0.9f));
            g.setFont (juce::FontOptions (16.0f));
            g.drawText (getButtonText(), getLocalBounds(), juce::Justification::centred);
        }
    };

    struct CardButton : public juce::Component
    {
        SoundLibrary& owner;
        int index = -1;
        PresetInfo info;
        float hover = 0.0f;
        HeartButton favBtn;

        explicit CardButton (SoundLibrary& o) : owner (o)
        {
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
            favBtn.setButtonText (juce::CharPointer_UTF8 ("\xe2\x99\xa1"));
            favBtn.onClick = [safe = juce::Component::SafePointer<CardButton> (this)] {
                if (safe == nullptr) return;
                const int idx = safe->index;
                const bool rebuildFav = safe->owner.activeCollection_.equalsIgnoreCase ("Favorites");
                if (safe->owner.onFavorite)
                    safe->owner.onFavorite (idx);
                if (rebuildFav)
                {
                    juce::MessageManager::callAsync ([ow = &safe->owner] {
                        ow->rebuild();
                    });
                }
                else if (safe != nullptr)
                {
                    safe->refreshFav();
                }
            };
            addAndMakeVisible (favBtn);
        }

        void refreshFav()
        {
            const bool fav = owner.presets_.isFavorite (info.name);
            favBtn.setButtonText (fav ? juce::CharPointer_UTF8 ("\xe2\x99\xa5")
                                      : juce::CharPointer_UTF8 ("\xe2\x99\xa1"));
            auto* laf = owner.laf_;
            favBtn.onCol = laf != nullptr ? laf->mint() : juce::Colour (0xff00f0ff);
            favBtn.offCol = laf != nullptr ? laf->textSecondary() : juce::Colour (0xff8a8a8a);
            favBtn.repaint();
        }

        void resized() override
        {
            // Heart sits on the meta row (below art), top-right of card body
            favBtn.setBounds (getWidth() - 30, 54, 26, 24);
        }

        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat().reduced (2.5f);
            auto* laf = owner.laf_;
            const bool sel = owner.selected_ == index;
            if (laf != nullptr)
                laf->drawCard (g, b, sel, hover > 0.2f, 10.0f);

            auto art = b.removeFromTop (52.0f).reduced (8.0f, 6.0f);
            paintCover (g, art);

            g.setColour (laf != nullptr ? laf->textPrimary() : juce::Colours::white);
            g.setFont (laf != nullptr ? laf->uiFont (14.0f, true) : juce::FontOptions (14.0f));
            g.drawText (info.name, (int) b.getX() + 8, (int) b.getY() + 2, (int) b.getWidth() - 36, 18,
                        juce::Justification::centredLeft, true);

            auto meta = juce::Rectangle<float> (b.getX() + 8.0f, b.getY() + 22.0f, b.getWidth() - 40.0f, 18.0f);
            const auto moodW = juce::jmin (96.0f, meta.getWidth() * 0.45f);
            auto moodR = meta.removeFromLeft (moodW);
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.22f));
            g.fillRoundedRectangle (moodR, 7.0f);
            g.setColour (laf != nullptr ? laf->textPrimary() : juce::Colours::white);
            g.setFont (laf != nullptr ? laf->uiFont (11.0f, true) : juce::FontOptions (11.0f));
            g.drawText (info.mood, moodR.reduced (4.0f, 0.0f), juce::Justification::centred, true);

            meta.removeFromLeft (6.0f);
            g.setColour (laf != nullptr ? laf->textSecondary() : juce::Colours::grey);
            g.setFont (laf != nullptr ? laf->uiFont (11.0f, true) : juce::FontOptions (11.0f));
            const auto kind = info.kind == LibraryItemKind::Wavetable ? "Wavetable"
                            : info.kind == LibraryItemKind::Sample ? "Sample" : info.category;
            g.drawText (kind, meta, juce::Justification::centredLeft, true);

            if (hover > 0.35f)
            {
                auto badge = juce::Rectangle<float> (art.getRight() - 48.0f, art.getBottom() - 20.0f, 44.0f, 16.0f);
                g.setColour ((laf != nullptr ? laf->ember() : juce::Colours::white).withAlpha (0.92f));
                g.fillRoundedRectangle (badge, 5.0f);
                g.setColour (laf != nullptr && laf->isLightTheme() ? juce::Colours::white : juce::Colours::black);
                g.setFont (laf != nullptr ? laf->uiFont (9.5f, true) : juce::FontOptions (9.5f));
                g.drawText ("PLAY", badge, juce::Justification::centred);
            }

            auto wave = b.removeFromBottom (14.0f).reduced (8.0f, 2.0f);
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
            juce::ColourGradient grad (c1.withAlpha (0.7f), art.getTopLeft(),
                                       c2, art.getBottomRight(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (art, 8.0f);

            juce::Path wave;
            for (int i = 0; i < 28; ++i)
            {
                const float u = (float) i / 27.0f;
                const float x = art.getX() + u * art.getWidth();
                const float y = art.getCentreY()
                    + std::sin (u * 6.28f * (1.2f + (h % 3)) + owner.phase_)
                      * art.getHeight() * 0.28f * (0.45f + info.energy);
                if (i == 0) wave.startNewSubPath (x, y);
                else wave.lineTo (x, y);
            }
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.75f));
            g.strokePath (wave, juce::PathStrokeType (1.8f));
        }

        void paintWave (juce::Graphics& g, juce::Rectangle<float> wave)
        {
            auto* laf = owner.laf_;
            g.setColour ((laf != nullptr ? laf->mint() : juce::Colours::cyan).withAlpha (0.55f));
            juce::Path p;
            for (int i = 0; i < 36; ++i)
            {
                const float u = (float) i / 35.0f;
                const float seed = (float) ((info.artworkSeed >> (i % 8)) & 7) / 7.0f;
                const float hh = (0.2f + seed * 0.75f) * wave.getHeight();
                const float x = wave.getX() + u * wave.getWidth();
                const float y = wave.getBottom() - hh;
                if (i == 0) p.startNewSubPath (x, y);
                else p.lineTo (x, y);
            }
            g.strokePath (p, juce::PathStrokeType (1.2f));
        }

        void mouseEnter (const juce::MouseEvent&) override { hover = 1.0f; repaint(); }
        void mouseExit (const juce::MouseEvent&) override { hover = 0.0f; repaint(); }

        void mouseDown (const juce::MouseEvent& e) override
        {
            // Heart is its own button — card click only loads/auditions
            if (favBtn.getBounds().contains (e.getPosition()))
                return;
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

    void rebuild()
    {
        visible_ = presets_.filteredIndices (activeCollection_, search.getText(),
                                             activeCollection_.equalsIgnoreCase ("Favorites"));

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
            card->refreshFav();
            grid.addAndMakeVisible (*card);
            grid.cards.push_back (std::move (card));
        }
        layoutGrid();
        repaint();
    }

    void layoutGrid()
    {
        // Prefer one readable column in the narrow browser; two when wide enough
        const int cols = viewport.getWidth() >= 300 ? 2 : 1;
        const int cardW = juce::jmax (140, (viewport.getWidth() - 6) / cols);
        const int cardH = 118;
        int i = 0;
        for (auto& c : grid.cards)
        {
            c->setBounds ((i % cols) * cardW + 1, (i / cols) * cardH + 1, cardW - 2, cardH - 2);
            ++i;
        }
        const int rows = (int) ((grid.cards.size() + cols - 1) / juce::jmax (1, cols));
        grid.setSize (viewport.getWidth(), juce::jmax (viewport.getHeight(), rows * cardH + 4));
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
    juce::ComboBox sortBox;
    juce::Label hint;
    juce::OwnedArray<juce::TextButton> chips;
    ChipWrap chipWrap;
    juce::Viewport viewport;
    GridComp grid { *this };
    std::vector<int> visible_;
    juce::String activeCollection_ { "All" };
    juce::StringArray collections_ { "All", "Favorites", "Bass", "Pads", "Leads", "Keys",
                                     "Vocals", "Textures", "Cinematic", "Experimental", "FX" };
    int selected_ = -1;
    float energy_ = 0.0f;
    float phase_ = 0.0f;
};
