#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ScorionLookAndFeel.h"
#include "assets/ResourcePaths.h"
#include <functional>
#include <vector>

/** Always-visible vocals/samples browser for the single canvas. */
class ContentBrowser : public juce::Component,
                       private juce::ListBoxModel
{
public:
    enum class Mode { Vocals, Samples };

    ContentBrowser()
    {
        vocalsTab.setClickingTogglesState (true);
        samplesTab.setClickingTogglesState (true);
        vocalsTab.setRadioGroupId (4401);
        samplesTab.setRadioGroupId (4401);
        vocalsTab.setToggleState (true, juce::dontSendNotification);
        vocalsTab.onClick = [this] { setMode (Mode::Vocals); };
        samplesTab.onClick = [this] { setMode (Mode::Samples); };
        addAndMakeVisible (vocalsTab);
        addAndMakeVisible (samplesTab);

        list.setModel (this);
        list.setRowHeight (48);
        list.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (list);
        reload();
    }

    void setLookAndFeelRef (ScorionLookAndFeel* laf) { laf_ = laf; }
    void setShowInternalTabs (bool show)
    {
        showTabs_ = show;
        vocalsTab.setVisible (show);
        samplesTab.setVisible (show);
        resized();
    }
    void setMode (Mode m) { mode_ = m; vocalsTab.setToggleState (m == Mode::Vocals, juce::dontSendNotification);
                            samplesTab.setToggleState (m == Mode::Samples, juce::dontSendNotification); reload(); }
    std::function<void (const juce::String& id, const juce::File& file)> onSelect;

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->labelFont() : juce::FontOptions (10.0f));
        g.drawText (mode_ == Mode::Vocals ? "VOCAL ASSETS" : "SAMPLE ASSETS", 8, 4, 120, 12, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->ember() : juce::Colours::orange);
        g.setFont (laf_ != nullptr ? laf_->uiFont (11.0f) : juce::FontOptions (11.0f));
        g.drawText (juce::String (items_.size()) + " assets",
                    (int) b.getRight() - 84, 4, 76, 12, juce::Justification::centredRight);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (8);
        a.removeFromTop (18);
        if (showTabs_)
        {
            auto tabs = a.removeFromTop (28);
            vocalsTab.setBounds (tabs.removeFromLeft (tabs.getWidth() / 2).reduced (2));
            samplesTab.setBounds (tabs.reduced (2));
            a.removeFromTop (6);
        }
        list.setBounds (a);
    }

    int getNumRows() override { return (int) items_.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (! juce::isPositiveAndBelow (row, (int) items_.size())) return;
        const auto& it = items_[(size_t) row];
        auto r = juce::Rectangle<float> (4.0f, 3.0f, (float) width - 8.0f, (float) height - 6.0f);
        if (selected)
        {
            g.setColour ((laf_ != nullptr ? laf_->ember() : juce::Colours::orange).withAlpha (0.22f));
            g.fillRoundedRectangle (r, 8.0f);
        }
        g.setColour (laf_ != nullptr ? laf_->textPrimary() : juce::Colours::white);
        g.setFont (laf_ != nullptr ? laf_->uiFont (12.5f, true) : juce::FontOptions (12.0f));
        g.drawText (it.name, 12, 6, width - 20, 18, juce::Justification::centredLeft);
        g.setColour (laf_ != nullptr ? laf_->textSecondary() : juce::Colours::grey);
        g.setFont (laf_ != nullptr ? laf_->uiFont (10.0f) : juce::FontOptions (10.0f));
        g.drawText (it.meta, 12, 24, width - 20, 14, juce::Justification::centredLeft);
    }

    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        if (! juce::isPositiveAndBelow (row, (int) items_.size())) return;
        if (onSelect) onSelect (items_[(size_t) row].id, items_[(size_t) row].file);
    }

    void reload()
    {
        items_.clear();
        if (mode_ == Mode::Vocals)
        {
            auto dir = ResourcePaths::factoryWavetables();
            for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.wtbin"))
            {
                const auto n = f.getFileName().toLowerCase();
                if (n.contains ("vox") || n.contains ("choir") || n.contains ("talk")
                    || n.contains ("formant") || n.contains ("ooh") || n.contains ("vocal"))
                    items_.push_back ({ f.getFileNameWithoutExtension().replaceCharacter ('_', ' '),
                                        f.getFileName(), "Wavetable", f });
            }
            auto pdir = ResourcePaths::factoryPresets();
            for (auto& f : pdir.findChildFiles (juce::File::findFiles, false, "*.json"))
            {
                auto txt = f.loadFileAsString();
                if (txt.containsIgnoreCase ("\"Vocal"))
                    items_.push_back ({ f.getFileNameWithoutExtension().replaceCharacter ('_', ' '),
                                        "Preset", "Vocal preset", f });
            }
        }
        else
        {
            auto dir = ResourcePaths::factorySamples();
            for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.wav"))
                items_.push_back ({ f.getFileNameWithoutExtension().replaceCharacter ('_', ' '),
                                    f.getFileName(), "Sample 44.1k", f });
        }
        if (items_.empty())
            items_.push_back ({ "No assets", "", "Generate factory content", {} });
        list.updateContent();
        repaint();
    }

private:
    struct Item { juce::String name, id, meta; juce::File file; };
    std::vector<Item> items_;
    juce::ListBox list;
    juce::TextButton vocalsTab { "Vocals" }, samplesTab { "Samples" };
    ScorionLookAndFeel* laf_ = nullptr;
    Mode mode_ = Mode::Vocals;
    bool showTabs_ = true;
};
