#include "ui/ScorionLookAndFeel.h"
#include "BinaryData.h"
#include <cmath>

ScorionLookAndFeel::ScorionLookAndFeel()
{
    loadFonts();
    atmosphere_ = juce::Colour (0xff00F0FF);
    setNamedTheme (NamedTheme::MonoNeon);
}

void ScorionLookAndFeel::loadFonts()
{
    spaceBold_ = juce::Typeface::createSystemTypefaceFor (BinaryData::SpaceGroteskBold_ttf,
                                                          BinaryData::SpaceGroteskBold_ttfSize);
    spaceSemi_ = juce::Typeface::createSystemTypefaceFor (BinaryData::SpaceGroteskSemiBold_ttf,
                                                          BinaryData::SpaceGroteskSemiBold_ttfSize);
    manropeRegular_ = juce::Typeface::createSystemTypefaceFor (BinaryData::ManropeRegular_ttf,
                                                               BinaryData::ManropeRegular_ttfSize);
    manropeMedium_ = juce::Typeface::createSystemTypefaceFor (BinaryData::ManropeMedium_ttf,
                                                              BinaryData::ManropeMedium_ttfSize);
    manropeSemi_ = juce::Typeface::createSystemTypefaceFor (BinaryData::ManropeSemiBold_ttf,
                                                            BinaryData::ManropeSemiBold_ttfSize);
}

juce::Typeface::Ptr ScorionLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    const auto style = f.getTypefaceStyle();
    const bool bold = style.containsIgnoreCase ("Bold") || f.isBold();
    const bool medium = style.containsIgnoreCase ("Medium") || style.containsIgnoreCase ("Semi");
    if (bold && spaceBold_ != nullptr) return spaceBold_;
    if (medium && manropeMedium_ != nullptr) return manropeMedium_;
    if (manropeRegular_ != nullptr) return manropeRegular_;
    return LookAndFeel_V4::getTypefaceForFont (f);
}

juce::Font ScorionLookAndFeel::brandFont (float size) const
{
    if (spaceBold_ != nullptr)
        return juce::Font (juce::FontOptions (spaceBold_).withHeight (size * uiScale_));
    return juce::Font (juce::FontOptions (size * uiScale_));
}

juce::Font ScorionLookAndFeel::uiFont (float size, bool medium) const
{
    auto tf = medium ? (manropeMedium_ != nullptr ? manropeMedium_ : manropeRegular_)
                     : manropeRegular_;
    if (tf != nullptr)
        return juce::Font (juce::FontOptions (tf).withHeight (size * uiScale_));
    return juce::Font (juce::FontOptions (size * uiScale_));
}

juce::Font ScorionLookAndFeel::labelFont() const { return uiFont (12.5f, true); }
juce::Font ScorionLookAndFeel::valueFont (float size) const { return uiFont (size, false); }
juce::Font ScorionLookAndFeel::titleFont (float size) const { return uiFont (size, true); }

void ScorionLookAndFeel::setNamedTheme (NamedTheme t)
{
    namedTheme_ = t;
    danger_ = juce::Colour (0xffFF3B5C);
    success_ = juce::Colour (0xff39FF14);
    warning_ = juce::Colour (0xffFFE566);
    border_ = juce::Colours::white.withAlpha (0.10f);

    switch (t)
    {
        case NamedTheme::MonoNeon:
            bg_ = juce::Colour (0xff050505);
            surface_ = juce::Colour (0xff0C0C0C);
            panel_ = juce::Colour (0xff121212);
            card_ = juce::Colour (0xff181818);
            surfaceHi_ = juce::Colour (0xff1E1E1E);
            ember_ = juce::Colour (0xffFFFFFF);
            emberDim_ = juce::Colour (0xffB8B8B8);
            mint_ = juce::Colour (0xff00F0FF);
            violet_ = mint_;
            metal_ = juce::Colour (0xff707070);
            textPrimary_ = juce::Colour (0xffFAFAFA);
            textSecondary_ = juce::Colour (0xffA0A0A0);
            atmosphere_ = mint_;
            break;

        case NamedTheme::ObsidianFrost:
            bg_ = juce::Colour (0xff06080C);
            surface_ = juce::Colour (0xff0C1016);
            panel_ = juce::Colour (0xff121820);
            card_ = juce::Colour (0xff1A222C);
            surfaceHi_ = juce::Colour (0xff222C38);
            ember_ = juce::Colour (0xffF2F6FA);
            emberDim_ = juce::Colour (0xffB0B8C0);
            mint_ = juce::Colour (0xff7EB8FF);
            violet_ = mint_;
            metal_ = juce::Colour (0xff6A7480);
            textPrimary_ = juce::Colour (0xffF4F7FA);
            textSecondary_ = juce::Colour (0xff9AA4B0);
            atmosphere_ = mint_;
            break;

        case NamedTheme::PureMono:
            bg_ = juce::Colour (0xff000000);
            surface_ = juce::Colour (0xff0A0A0A);
            panel_ = juce::Colour (0xff111111);
            card_ = juce::Colour (0xff171717);
            surfaceHi_ = juce::Colour (0xff1F1F1F);
            ember_ = juce::Colour (0xffFFFFFF);
            emberDim_ = juce::Colour (0xffC0C0C0);
            mint_ = juce::Colour (0xffE0E0E0); // no colour — soft white
            violet_ = mint_;
            metal_ = juce::Colour (0xff666666);
            textPrimary_ = juce::Colour (0xffFFFFFF);
            textSecondary_ = juce::Colour (0xff9A9A9A);
            atmosphere_ = juce::Colour (0xffFFFFFF);
            border_ = juce::Colours::white.withAlpha (0.14f);
            break;

        case NamedTheme::NoirPhosphor:
            bg_ = juce::Colour (0xff030503);
            surface_ = juce::Colour (0xff080B08);
            panel_ = juce::Colour (0xff0E120E);
            card_ = juce::Colour (0xff141A14);
            surfaceHi_ = juce::Colour (0xff1A221A);
            ember_ = juce::Colour (0xffE8FFE8);
            emberDim_ = juce::Colour (0xffA8C8A8);
            mint_ = juce::Colour (0xff39FF14);
            violet_ = mint_;
            metal_ = juce::Colour (0xff5A6A5A);
            textPrimary_ = juce::Colour (0xffE8FFE8);
            textSecondary_ = juce::Colour (0xff7A9A7A);
            atmosphere_ = mint_;
            break;

        case NamedTheme::IvoryCircuit:
            bg_ = juce::Colour (0xffE8E6E2);
            surface_ = juce::Colour (0xffF2F0EC);
            panel_ = juce::Colour (0xffFAF8F4);
            card_ = juce::Colour (0xffFFFFFF);
            surfaceHi_ = juce::Colour (0xffFFFFFF);
            ember_ = juce::Colour (0xff111111);
            emberDim_ = juce::Colour (0xff333333);
            mint_ = juce::Colour (0xff0088AA);
            violet_ = mint_;
            metal_ = juce::Colour (0xff888888);
            textPrimary_ = juce::Colour (0xff111111);
            textSecondary_ = juce::Colour (0xff555555);
            atmosphere_ = mint_;
            border_ = juce::Colours::black.withAlpha (0.10f);
            danger_ = juce::Colour (0xffCC2244);
            break;
    }

    applyColourIds();
}

void ScorionLookAndFeel::applyColourIds()
{
    setColour (juce::ResizableWindow::backgroundColourId, bg_);
    setColour (juce::Slider::thumbColourId, textPrimary_);
    setColour (juce::Slider::rotarySliderFillColourId, ember_);
    setColour (juce::Slider::rotarySliderOutlineColourId, metal_.withAlpha (0.35f));
    setColour (juce::Slider::trackColourId, ember_);
    setColour (juce::Slider::backgroundColourId, surfaceHi_);
    setColour (juce::Slider::textBoxTextColourId, textSecondary_);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, textPrimary_);
    setColour (juce::TextButton::buttonColourId, surfaceHi_);
    setColour (juce::TextButton::textColourOffId, textPrimary_);
    setColour (juce::TextButton::textColourOnId, isLightTheme() ? juce::Colours::white : bg_);
    setColour (juce::ComboBox::backgroundColourId, surfaceHi_);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::textColourId, textPrimary_);
    setColour (juce::ComboBox::arrowColourId, mint_);
    setColour (juce::TextEditor::backgroundColourId, surfaceHi_);
    setColour (juce::TextEditor::textColourId, textPrimary_);
    setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::TextEditor::focusedOutlineColourId, mint_.withAlpha (0.65f));
    setColour (juce::TextEditor::highlightColourId, mint_.withAlpha (0.28f));
    setColour (juce::PopupMenu::backgroundColourId, panel_);
    setColour (juce::PopupMenu::textColourId, textPrimary_);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, mint_.withAlpha (0.28f));
    setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ListBox::textColourId, textPrimary_);
    setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId, mint_.withAlpha (0.55f));
    setColour (juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
    setColour (juce::TooltipWindow::backgroundColourId, panel_);
    setColour (juce::TooltipWindow::textColourId, textPrimary_);
    setColour (juce::TooltipWindow::outlineColourId, border_);

    setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
               isLightTheme() ? juce::Colour (0xffFFFFFF) : juce::Colour (0xffF0F0F0));
    setColour (juce::MidiKeyboardComponent::blackNoteColourId,
               isLightTheme() ? juce::Colour (0xff222222) : juce::Colour (0xff111111));
    setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff2A2A2A));
    setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, mint_.withAlpha (0.22f));
    setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, ember_.withAlpha (0.45f));
    setColour (juce::MidiKeyboardComponent::textLabelColourId, textSecondary_);
    setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.45f));
}

void ScorionLookAndFeel::drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds, float corner) const
{
    // Soft drop shadow
    g.setColour (juce::Colours::black.withAlpha (isLightTheme() ? 0.12f : 0.55f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 3.0f), corner);

    // Panel fill with subtle top-light gradient
    {
        juce::ColourGradient grad (panel_.brighter (isLightTheme() ? 0.02f : 0.06f),
                                   bounds.getCentreX(), bounds.getY(),
                                   panel_.darker (isLightTheme() ? 0.04f : 0.08f),
                                   bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, corner);
    }
    g.setColour (border_);
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);
    g.setColour (mint_.withAlpha (0.10f));
    g.drawRoundedRectangle (bounds.reduced (1.5f), corner - 1.0f, 1.0f);
}

void ScorionLookAndFeel::drawInsetWell (juce::Graphics& g, juce::Rectangle<float> bounds, float corner) const
{
    g.setColour (juce::Colours::black.withAlpha (isLightTheme() ? 0.08f : 0.65f));
    g.fillRoundedRectangle (bounds.translated (0, 1), corner);
    {
        juce::ColourGradient grad (bg_.darker (0.2f), bounds.getCentreX(), bounds.getY(),
                                   surface_, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, corner);
    }
    g.setColour (border_);
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);
}

void ScorionLookAndFeel::drawCard (juce::Graphics& g, juce::Rectangle<float> bounds, bool selected, bool hovered, float corner) const
{
    g.setColour (juce::Colours::black.withAlpha (isLightTheme() ? 0.08f : 0.45f));
    g.fillRoundedRectangle (bounds.translated (0, hovered || selected ? 1.0f : 3.0f), corner);

    {
        juce::ColourGradient grad (card_.brighter (hovered ? 0.08f : 0.03f),
                                   bounds.getCentreX(), bounds.getY(),
                                   card_.darker (0.06f), bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, corner);
    }

    if (selected)
    {
        g.setColour (mint_.withAlpha (0.9f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.8f);
    }
    else
    {
        g.setColour (hovered ? mint_.withAlpha (0.4f) : border_);
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, hovered ? 1.4f : 1.0f);
    }
}

void ScorionLookAndFeel::drawPill (juce::Graphics& g, juce::Rectangle<float> bounds, bool active, bool emberAccent) const
{
    const float r = juce::jmin (14.0f, bounds.getHeight() * 0.5f);
    if (active)
    {
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (bounds.translated (0, 2), r);
        g.setColour (emberAccent ? ember_ : mint_);
        g.fillRoundedRectangle (bounds, r);
    }
    else
    {
        g.setColour (surfaceHi_);
        g.fillRoundedRectangle (bounds, r);
        g.setColour (border_);
        g.drawRoundedRectangle (bounds.reduced (0.5f), r, 1.0f);
    }
}

void ScorionLookAndFeel::drawSectionLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text) const
{
    g.setColour (textPrimary_.withAlpha (0.85f));
    g.setFont (titleFont (13.0f));
    g.drawText (text.toUpperCase(), bounds, juce::Justification::centredLeft);
}

void ScorionLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.0f);
    const bool hero = slider.getName() == "hero" || width >= 92;
    const bool mod = slider.getName() == "mod" || slider.getComponentID() == "mod"
                     || slider.getComponentID().contains ("modring");
    const float scalePad = showKnobScales_ ? (hero ? 14.0f : 12.0f) : 4.0f;
    const float size = juce::jmin (bounds.getWidth(), bounds.getHeight() - scalePad);
    auto knob = juce::Rectangle<float> (bounds.getCentreX() - size * 0.5f, bounds.getY() + 2.0f, size, size);
    const auto centre = knob.getCentre();
    const float radius = size * 0.5f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto accent = mod ? mint_ : ember_;
    const bool active = slider.isMouseOverOrDragging() || slider.isMouseButtonDown();
    const float glow = active ? 1.0f : 0.0f;

    // Soft contact shadow
    g.setColour (juce::Colours::black.withAlpha (isLightTheme() ? 0.08f : 0.35f));
    g.fillEllipse (centre.x - radius + 1.5f, centre.y - radius + 3.0f, radius * 2.0f, radius * 2.0f);

    // Flat sober body
    {
        const auto bodyTop = surfaceHi_.interpolatedWith (textPrimary_, isLightTheme() ? 0.04f : 0.06f);
        const auto bodyBot = surface_.darker (0.12f);
        juce::ColourGradient body (bodyTop, centre.x, centre.y - radius,
                                   bodyBot, centre.x, centre.y + radius, false);
        g.setGradientFill (body);
        g.fillEllipse (knob);
    }

    // Thin rim
    g.setColour (border_.withAlpha (0.9f));
    g.drawEllipse (knob.reduced (0.5f), 1.0f);

    // Quiet track (full travel)
    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                             rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (textPrimary_.withAlpha (0.12f));
        g.strokePath (track, juce::PathStrokeType (hero ? 1.6f : 1.25f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    // Optional sparse scale ticks (sober)
    if (showKnobScales_)
    {
        for (int i = 0; i <= 10; ++i)
        {
            const float t = (float) i / 10.0f;
            const float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            const float r0 = radius - (hero ? 8.0f : 6.5f);
            const float r1 = radius - 3.5f;
            g.setColour (textPrimary_.withAlpha (i % 5 == 0 ? 0.35f : 0.14f));
            g.drawLine (centre.x + std::cos (a) * r0, centre.y + std::sin (a) * r0,
                        centre.x + std::cos (a) * r1, centre.y + std::sin (a) * r1, 1.0f);
            if (hero && (i == 0 || i == 5 || i == 10))
            {
                const float lr = radius + 7.0f;
                g.setColour (textSecondary_.withAlpha (0.75f));
                g.setFont (valueFont (8.0f));
                g.drawText (juce::String (i),
                            (int) (centre.x + std::cos (a) * lr) - 8,
                            (int) (centre.y + std::sin (a) * lr) - 5,
                            16, 10, juce::Justification::centred, false);
            }
        }
    }

    // Single indicator line — sober at rest, colour glow on hover / hold
    {
        const float rInner = radius * (hero ? 0.18f : 0.20f);
        const float rOuter = radius * (hero ? 0.72f : 0.70f);
        const float x0 = centre.x + std::cos (angle) * rInner;
        const float y0 = centre.y + std::sin (angle) * rInner;
        const float x1 = centre.x + std::cos (angle) * rOuter;
        const float y1 = centre.y + std::sin (angle) * rOuter;
        const float lineW = hero ? 2.0f : 1.6f;

        if (glow > 0.0f)
        {
            g.setColour (accent.withAlpha (0.22f + 0.18f * glow));
            g.drawLine (x0, y0, x1, y1, lineW + 5.0f);
            g.setColour (accent.withAlpha (0.45f));
            g.drawLine (x0, y0, x1, y1, lineW + 2.2f);
        }

        // Core line: muted white at rest, accent when active
        g.setColour (active ? accent.withAlpha (0.98f)
                            : textPrimary_.withAlpha (isLightTheme() ? 0.55f : 0.72f));
        g.drawLine (x0, y0, x1, y1, lineW);

        // Tiny tip
        const float tip = hero ? 2.4f : 2.0f;
        g.setColour (active ? accent : textPrimary_.withAlpha (0.65f));
        g.fillEllipse (x1 - tip, y1 - tip, tip * 2.0f, tip * 2.0f);
        if (active)
        {
            g.setColour (accent.withAlpha (0.35f));
            g.fillEllipse (x1 - tip * 2.2f, y1 - tip * 2.2f, tip * 4.4f, tip * 4.4f);
        }
    }

    // Quiet hub
    {
        const float cr = hero ? 3.2f : 2.6f;
        g.setColour (surface_.darker (0.2f));
        g.fillEllipse (centre.x - cr, centre.y - cr, cr * 2.0f, cr * 2.0f);
        g.setColour (textPrimary_.withAlpha (active ? 0.45f : 0.22f));
        g.drawEllipse (centre.x - cr, centre.y - cr, cr * 2.0f, cr * 2.0f, 1.0f);
    }

    if (active)
    {
        g.setColour (accent.withAlpha (0.95f));
        g.setFont (valueFont (10.5f));
        g.drawText (slider.getTextFromValue (slider.getValue()),
                    bounds.removeFromBottom (13.0f), juce::Justification::centred);
    }
}

void ScorionLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour&,
                                               bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const auto text = button.getButtonText();
    const bool primary = button.getToggleState()
                         || text.equalsIgnoreCase ("Init")
                         || text.equalsIgnoreCase ("Master");
    const bool danger = text.equalsIgnoreCase ("Panic") || text.equalsIgnoreCase ("Bypass");
    const float r = 14.0f;

    if (primary && ! danger)
    {
        drawPill (g, bounds, true, true);
        return;
    }

    auto base = danger ? danger_.darker (0.15f) : surfaceHi_;
    if (down) base = base.brighter (0.08f);
    else if (highlighted) base = base.brighter (0.1f);

    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRoundedRectangle (bounds.translated (0, 2), r);
    g.setColour (base);
    g.fillRoundedRectangle (bounds, r);
    g.setColour (danger ? danger_.withAlpha (0.8f) : (highlighted ? mint_.withAlpha (0.45f) : border_));
    g.drawRoundedRectangle (bounds.reduced (0.5f), r, 1.0f);
}

void ScorionLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox&)
{
    auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRoundedRectangle (bounds.translated (0, 2), 14.0f);
    g.setColour (surfaceHi_);
    g.fillRoundedRectangle (bounds, 14.0f);
    g.setColour (border_);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 14.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) width - 14.0f;
    const float ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 4.0f, ay - 2.5f, ax + 4.0f, ay - 2.5f, ax, ay + 3.5f);
    g.setColour (mint_);
    g.fillPath (arrow);
}

void ScorionLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float minSliderPos, float maxSliderPos,
                                           juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal && style != juce::Slider::LinearBar)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto track = juce::Rectangle<float> ((float) x, (float) y + (float) height * 0.5f - 2.0f,
                                         (float) width, 4.0f);
    g.setColour (surfaceHi_);
    g.fillRoundedRectangle (track, 2.0f);

    auto fill = track.withWidth (juce::jmax (4.0f, sliderPos - (float) x));
    g.setColour (mint_);
    g.fillRoundedRectangle (fill, 2.0f);

    g.setColour (ember_);
    g.fillEllipse (sliderPos - 6.0f, (float) y + (float) height * 0.5f - 6.0f, 12.0f, 12.0f);
}
