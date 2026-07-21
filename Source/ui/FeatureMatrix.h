#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "assets/ResourcePaths.h"
#include <vector>

/** Scrollable flagship feature wall (100+ capabilities). */
class FeatureMatrix : public juce::Component,
                      private juce::ListBoxModel
{
public:
    FeatureMatrix()
    {
        load();
        list.setModel (this);
        list.setRowHeight (52);
        list.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (list);
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf)
    {
        laf_ = laf;
        list.setColour (juce::ListBox::textColourId,
                        laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white);
    }

    int count() const noexcept { return (int) rows_.size(); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (laf_ != nullptr)
            laf_->drawPanel (g, b, 20.0f);

        auto head = b.removeFromTop (56.0f).reduced (18.0f, 14.0f);
        g.setColour (laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white);
        g.setFont (juce::FontOptions (18.0f).withStyle ("Bold"));
        g.drawText ("Flagship Feature Hub", head, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->accent() : juce::Colours::cyan);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText (juce::String (rows_.size()) + " capabilities", head, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (12);
        a.removeFromTop (48);
        list.setBounds (a);
    }

    int getNumRows() override { return (int) rows_.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (! juce::isPositiveAndBelow (row, (int) rows_.size())) return;
        const auto& r = rows_[(size_t) row];
        auto bounds = juce::Rectangle<float> (6.0f, 4.0f, (float) width - 12.0f, (float) height - 8.0f);
        if (selected || (row % 2 == 0))
        {
            g.setColour (juce::Colours::white.withAlpha (selected ? 0.08f : 0.03f));
            g.fillRoundedRectangle (bounds, 12.0f);
        }
        g.setColour ((laf_ != nullptr ? laf_->accent() : juce::Colours::cyan).withAlpha (0.85f));
        g.fillEllipse (bounds.getX() + 12.0f, bounds.getCentreY() - 3.0f, 6.0f, 6.0f);

        g.setColour (laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white);
        g.setFont (juce::FontOptions (13.0f).withStyle ("Medium"));
        g.drawText (r.title, (int) bounds.getX() + 28, (int) bounds.getY() + 6, width - 48, 18,
                    juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (juce::FontOptions (11.0f));
        g.drawText (r.group + "  ·  " + r.blurb, (int) bounds.getX() + 28, (int) bounds.getY() + 26,
                    width - 48, 16, juce::Justification::centredLeft);
    }

private:
    struct Row { juce::String group, title, blurb; };
    std::vector<Row> rows_;
    juce::ListBox list;
    ScorionLookAndFeel* laf_ = nullptr;

    void load()
    {
        rows_.clear();
        auto f = ResourcePaths::factoryRoot().getChildFile ("factory").getChildFile ("features.json");
        if (f.existsAsFile())
        {
            auto json = juce::JSON::parse (f.loadFileAsString());
            if (auto* root = json.getDynamicObject())
            {
                if (auto* arr = root->getProperty ("features").getArray())
                {
                    for (auto& v : *arr)
                    {
                        if (auto* o = v.getDynamicObject())
                            rows_.push_back ({ o->getProperty ("group").toString(),
                                               o->getProperty ("title").toString(),
                                               o->getProperty ("blurb").toString() });
                    }
                }
            }
        }
        if (rows_.empty())
        {
            for (int i = 1; i <= 108; ++i)
                rows_.push_back ({ "Core", "Capability " + juce::String (i), "Built-in flagship module" });
        }
    }
};
