#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Motion.h"

/** Black/white neon chrome with named premium themes. */
class ScorionLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class NamedTheme
    {
        MonoNeon = 0,      // void + white + cyan
        ObsidianFrost,     // void + white + ice blue
        PureMono,          // strict black / white
        NoirPhosphor,      // void + white + phosphor green
        IvoryCircuit       // light ivory + black + cyan
    };

    static constexpr int kNumThemes = 5;

    static juce::String themeName (NamedTheme t)
    {
        switch (t)
        {
            case NamedTheme::MonoNeon:      return "Mono Neon";
            case NamedTheme::ObsidianFrost: return "Obsidian Frost";
            case NamedTheme::PureMono:      return "Pure Mono";
            case NamedTheme::NoirPhosphor:  return "Noir Phosphor";
            case NamedTheme::IvoryCircuit:  return "Ivory Circuit";
        }
        return "Mono Neon";
    }

    static juce::String themeBlurb (NamedTheme t)
    {
        switch (t)
        {
            case NamedTheme::MonoNeon:      return "Void black, white neon, cyan mod";
            case NamedTheme::ObsidianFrost: return "Cold steel whites with ice-blue glow";
            case NamedTheme::PureMono:      return "Strict black & white, no colour accent";
            case NamedTheme::NoirPhosphor:  return "CRT phosphor green on deep black";
            case NamedTheme::IvoryCircuit:  return "Light ivory panels, ink black type";
        }
        return {};
    }

    ScorionLookAndFeel();

    void setNamedTheme (NamedTheme t);
    NamedTheme getNamedTheme() const noexcept { return namedTheme_; }
    void useShowModePalette() { setNamedTheme (NamedTheme::MonoNeon); }

    void setAtmosphereTint (juce::Colour c) { atmosphere_ = c; }
    void setShowKnobScales (bool on) { showKnobScales_ = on; }
    bool getShowKnobScales() const noexcept { return showKnobScales_; }
    void setUiScale (float s) { uiScale_ = juce::jlimit (0.85f, 1.5f, s); }
    float getUiScale() const noexcept { return uiScale_; }

    juce::Colour background() const noexcept { return bg_; }
    juce::Colour surface() const noexcept { return surface_; }
    juce::Colour surfaceHi() const noexcept { return surfaceHi_; }
    juce::Colour panel() const noexcept { return panel_; }
    juce::Colour card() const noexcept { return card_; }
    juce::Colour border() const noexcept { return border_; }
    juce::Colour accent() const noexcept { return ember_; }
    juce::Colour accentDim() const noexcept { return emberDim_; }
    juce::Colour accentBlue() const noexcept { return mint_; }
    juce::Colour mint() const noexcept { return mint_; }
    juce::Colour ember() const noexcept { return ember_; }
    juce::Colour violet() const noexcept { return violet_; }
    juce::Colour success() const noexcept { return success_; }
    juce::Colour warning() const noexcept { return warning_; }
    juce::Colour metal() const noexcept { return metal_; }
    juce::Colour danger() const noexcept { return danger_; }
    juce::Colour textPrimary() const noexcept { return textPrimary_; }
    juce::Colour textSecondary() const noexcept { return textSecondary_; }
    juce::Colour atmosphere() const noexcept { return atmosphere_; }
    bool isLightTheme() const noexcept { return namedTheme_ == NamedTheme::IvoryCircuit; }

    juce::Font brandFont (float size) const;
    juce::Font uiFont (float size, bool medium = false) const;
    juce::Font labelFont() const;
    juce::Font valueFont (float size = 11.0f) const;
    juce::Font titleFont (float size = 14.0f) const;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds, float corner = 14.0f) const;
    void drawInsetWell (juce::Graphics& g, juce::Rectangle<float> bounds, float corner = 12.0f) const;
    void drawPill (juce::Graphics& g, juce::Rectangle<float> bounds, bool active, bool emberAccent = true) const;
    void drawSectionLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text) const;
    void drawCard (juce::Graphics& g, juce::Rectangle<float> bounds, bool selected, bool hovered, float corner = 12.0f) const;

    juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override;

private:
    void loadFonts();
    void applyColourIds();

    NamedTheme namedTheme_ = NamedTheme::MonoNeon;
    bool showKnobScales_ = true;
    float uiScale_ = 1.0f;
    juce::Colour bg_, surface_, surfaceHi_, panel_, card_, border_;
    juce::Colour ember_, emberDim_, mint_, violet_, success_, warning_, metal_, danger_;
    juce::Colour textPrimary_, textSecondary_, atmosphere_;
    juce::Typeface::Ptr spaceBold_, spaceSemi_, manropeRegular_, manropeMedium_, manropeSemi_;
};
