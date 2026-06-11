#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "TextureGenerator.h"

//==============================================================================
// KNOB VISUAL STYLE
// Set on slider via properties
// Read by AnalogLookAndFeel::drawRotarySlider
//==============================================================================
enum class KnobVisualStyle
{
    DarkMetal   = 0,
    VintageDome = 1,
    SSLCompact  = 2,
    APISkirt    = 3
};

inline void setKnobVisualStyle(
    juce::Slider& slider,
    KnobVisualStyle style)
{
    slider.getProperties().set("knobStyle",
        static_cast<int>(style));
}

//==============================================================================
// PANEL TEXTURES
// All texture access guards against null TextureCache
// during shutdown period
//==============================================================================
namespace PanelTextures
{
    inline void drawBrushedMetal(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        juce::Colour tintColor,
        float scratchIntensity = 0.03f)
    {
        // Base tint fill
        g.setColour(tintColor);
        g.fillRoundedRectangle(area, 4.0f);

        // Texture overlay
        auto* tex = TextureCache::getInstance();
        if (tex != nullptr)
        {
            float brightness = tintColor.getBrightness();
            juce::Image* metalTex =
                (brightness > 0.5f)
                ? &tex->brushedMetalSilver
                : (brightness > 0.25f)
                    ? &tex->brushedMetalGrey
                    : &tex->brushedMetalDark;

            if (metalTex != nullptr
             && !metalTex->isNull())
            {
                g.setOpacity(0.35f);
                g.drawImage(*metalTex,
                    static_cast<int>(area.getX()),
                    static_cast<int>(area.getY()),
                    static_cast<int>(area.getWidth()),
                    static_cast<int>(area.getHeight()),
                    0, 0,
                    metalTex->getWidth(),
                    metalTex->getHeight());
                g.setOpacity(1.0f);
            }

            if (!tex->scratchOverlay.isNull())
            {
                g.setOpacity(scratchIntensity * 2.5f);
                g.drawImage(tex->scratchOverlay,
                    static_cast<int>(area.getX()),
                    static_cast<int>(area.getY()),
                    static_cast<int>(area.getWidth()),
                    static_cast<int>(area.getHeight()),
                    0, 0,
                    tex->scratchOverlay.getWidth(),
                    tex->scratchOverlay.getHeight());
                g.setOpacity(1.0f);
            }
        }

        // Top bevel - catches overhead light
        g.setColour(tintColor.brighter(0.08f));
        g.drawHorizontalLine(
            static_cast<int>(area.getY()) + 1,
            area.getX() + 3,
            area.getRight() - 3);

        // Bottom shadow
        g.setColour(tintColor.darker(0.15f));
        g.drawHorizontalLine(
            static_cast<int>(area.getBottom()) - 2,
            area.getX() + 3,
            area.getRight() - 3);

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(area, 4.0f, 1.0f);
    }

    inline void drawWrinkleFinish(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        juce::Colour baseColor)
    {
        g.setColour(baseColor);
        g.fillRoundedRectangle(area, 4.0f);

        auto* tex = TextureCache::getInstance();
        if (tex != nullptr
         && !tex->wrinkleFinish.isNull())
        {
            g.setOpacity(0.55f);
            g.drawImage(tex->wrinkleFinish,
                static_cast<int>(area.getX()),
                static_cast<int>(area.getY()),
                static_cast<int>(area.getWidth()),
                static_cast<int>(area.getHeight()),
                0, 0,
                tex->wrinkleFinish.getWidth(),
                tex->wrinkleFinish.getHeight());
            g.setOpacity(1.0f);
        }

        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(area, 4.0f, 1.0f);
    }

    inline void drawDust(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        float intensity = 0.008f)
    {
        auto* tex = TextureCache::getInstance();
        if (tex == nullptr
         || tex->dustOverlay.isNull())
            return;

        g.setOpacity(intensity * 6.0f);
        g.drawImage(tex->dustOverlay,
            static_cast<int>(area.getX()),
            static_cast<int>(area.getY()),
            static_cast<int>(area.getWidth()),
            static_cast<int>(area.getHeight()),
            0, 0,
            tex->dustOverlay.getWidth(),
            tex->dustOverlay.getHeight());
        g.setOpacity(1.0f);
    }

    inline void drawScrewHead(
        juce::Graphics& g,
        float cx, float cy,
        float rotation = 0.3f)
    {
        auto* tex = TextureCache::getInstance();
        if (tex == nullptr
         || tex->screwHead.isNull())
            return;

        int s = 12;
        juce::Graphics::ScopedSaveState save(g);
        juce::AffineTransform t =
            juce::AffineTransform::rotation(
                rotation, cx, cy);
        g.addTransform(t);
        g.drawImage(tex->screwHead,
            static_cast<int>(cx - s * 0.5f),
            static_cast<int>(cy - s * 0.5f),
            s, s,
            0, 0,
            tex->screwHead.getWidth(),
            tex->screwHead.getHeight());
    }

    inline void drawEngravedText(
        juce::Graphics& g,
        const juce::String& text,
        juce::Rectangle<int> bounds,
        juce::Justification just =
            juce::Justification::centred)
    {
        g.setFont(juce::Font(
            juce::FontOptions(10.0f)
            .withStyle("Bold")));

        // Shadow pass - depth of engraving
        g.setColour(juce::Colour(0xFF0A0A0A));
        g.drawText(text,
            bounds.translated(1, 1), just);

        // Gold fill pass
        g.setColour(juce::Colour(0xFFE8C878));
        g.drawText(text, bounds, just);
    }

    inline void drawWoodPanel(
        juce::Graphics& g,
        juce::Rectangle<float> area)
    {
        auto* tex = TextureCache::getInstance();

        if (tex == nullptr
         || tex->woodGrain.isNull())
        {
            // Fallback solid color
            g.setColour(juce::Colour(0xFF3C2A18));
            g.fillRect(area);
            return;
        }

        g.drawImage(tex->woodGrain,
            static_cast<int>(area.getX()),
            static_cast<int>(area.getY()),
            static_cast<int>(area.getWidth()),
            static_cast<int>(area.getHeight()),
            0, 0,
            tex->woodGrain.getWidth(),
            tex->woodGrain.getHeight());

        // Varnish sheen
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.05f),
            area.getX(), area.getY(),
            juce::Colours::transparentWhite,
            area.getRight(), area.getY(),
            false));
        g.fillRect(area);
    }
}

//==============================================================================
// ANALOG NEEDLE METER
// NO INTERNAL TIMER - eliminates timer race crash
// Editor drives physics via advancePhysics() at 30Hz
//==============================================================================
class AnalogNeedleMeter : public juce::Component
{
public:
    AnalogNeedleMeter() {}

    ~AnalogNeedleMeter() override {}

    void setGainReduction(float grDb)
    {
        targetAngle = juce::jlimit(0.0f, 1.0f,
            grDb / 20.0f);
    }

    void setModel(int m)
    {
        if (m != model)
        {
            model = m;
            repaint();
        }
    }

    // Called by editor timerCallback at 30Hz
    // Only repaints when needle actually moves
    void advancePhysics()
    {
        float force = (targetAngle - needleAngle)
            * 0.15f;
        needleVelocity += force;
        needleVelocity *= 0.78f;

        float newAngle = juce::jlimit(0.0f, 1.0f,
            needleAngle + needleVelocity);

        if (std::abs(newAngle - needleAngle) > 0.0002f)
        {
            needleAngle = newAngle;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto* tex = TextureCache::getInstance();
        auto bounds = getLocalBounds().toFloat();
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float cx = w * 0.5f;
        float pivotY = h * 0.88f;
        float adjustY = pivotY - h * 0.12f;

        // Model specific colors
        juce::Colour faceColor;
        juce::Colour needleCol;
        juce::Colour glowCol;

        switch (model)
        {
            case 0: // SSL
                faceColor = juce::Colour(0xFFF0F0F0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x2040C040);
                break;
            case 1: // Fairchild
                faceColor = juce::Colour(0xFFE8D8C0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20C8A838);
                break;
            case 2: // LA-2A
                faceColor = juce::Colour(0xFFE8E0D0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20D4900A);
                break;
            case 3: // 1176
                faceColor = juce::Colour(0xFF0A0A0A);
                needleCol = juce::Colour(0xFFD0D0D0);
                glowCol   = juce::Colour(0x20606060);
                break;
            case 4: // API
                faceColor = juce::Colour(0xFFF0F0F0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x154488FF);
                break;
            default:
                faceColor = juce::Colour(0xFFE8E0D0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20808060);
                break;
        }

        // Outer bezel
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(juce::Colour(0xFF303030));
        g.drawRoundedRectangle(bounds, 5.0f, 1.5f);
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(
            bounds.reduced(2.0f), 4.0f);

        auto faceRect = bounds.reduced(4.0f);

        // VU face image - model specific
        juce::Image* faceImg = nullptr;
        if (tex != nullptr)
        {
            switch (model)
            {
                case 3:
                    faceImg = &tex->vuFaceBlack;
                    break;
                case 0:
                case 4:
                    faceImg = &tex->vuFaceWhite;
                    break;
                default:
                    faceImg = &tex->vuFaceCream;
                    break;
            }
        }

        if (faceImg != nullptr
         && !faceImg->isNull())
        {
            g.drawImage(*faceImg,
                static_cast<int>(faceRect.getX()),
                static_cast<int>(faceRect.getY()),
                static_cast<int>(faceRect.getWidth()),
                static_cast<int>(faceRect.getHeight()),
                0, 0,
                faceImg->getWidth(),
                faceImg->getHeight());
        }
        else
        {
            g.setColour(faceColor);
            g.fillRoundedRectangle(faceRect, 3.0f);
        }

        // Backlight glow
        g.setColour(glowCol);
        g.fillRoundedRectangle(faceRect, 3.0f);

        // Needle
        float startAngle = -2.35f;
        float sweepAngle = 4.7f;
        float needleRot  = startAngle
            + needleAngle * sweepAngle;
        float needleLen  = h * 0.62f;

        float nx = cx + needleLen
            * std::cos(needleRot);
        float ny = adjustY + needleLen
            * std::sin(needleRot);

        // Shadow
        g.setColour(juce::Colour(0x18000000));
        g.drawLine(cx + 0.8f, adjustY + 0.8f,
                   nx + 0.8f, ny + 0.8f, 2.0f);

        // Tapered needle body
        float baseW  = 1.8f;
        float tipW   = 0.3f;
        float perpA  = needleRot
            + juce::MathConstants<float>::pi * 0.5f;
        float pcosA  = std::cos(perpA);
        float psinA  = std::sin(perpA);

        juce::Path needle;
        needle.startNewSubPath(
            cx    + baseW * pcosA,
            adjustY + baseW * psinA);
        needle.lineTo(
            nx + tipW * pcosA,
            ny + tipW * psinA);
        needle.lineTo(
            nx - tipW * pcosA,
            ny - tipW * psinA);
        needle.lineTo(
            cx    - baseW * pcosA,
            adjustY - baseW * psinA);
        needle.closeSubPath();

        g.setColour(needleCol);
        g.fillPath(needle);

        // Pivot cap
        float pivR = 5.0f;
        g.setColour(juce::Colour(0xFF383838));
        g.fillEllipse(cx - pivR, adjustY - pivR,
            pivR * 2, pivR * 2);
        g.setColour(juce::Colour(0xFF606060));
        g.drawEllipse(cx - pivR, adjustY - pivR,
            pivR * 2, pivR * 2, 0.8f);
        g.setColour(juce::Colour(0xFF505050));
        g.fillEllipse(cx - 2, adjustY - 2, 4, 4);

        // Glass reflection overlay
        if (tex != nullptr
         && !tex->glassReflection.isNull())
        {
            g.setOpacity(0.5f);
            g.drawImage(tex->glassReflection,
                static_cast<int>(faceRect.getX()),
                static_cast<int>(faceRect.getY()),
                static_cast<int>(faceRect.getWidth()),
                static_cast<int>(faceRect.getHeight()),
                0, 0,
                tex->glassReflection.getWidth(),
                tex->glassReflection.getHeight());
            g.setOpacity(1.0f);
        }

        // Outer glass edge
        g.setColour(juce::Colour(0xFF404040));
        g.drawRoundedRectangle(
            bounds.reduced(1.0f), 4.5f, 1.0f);
    }

private:
    float targetAngle    = 0.0f;
    float needleAngle    = 0.0f;
    float needleVelocity = 0.0f;
    int   model          = 0;
};

//==============================================================================
// LED LADDER METER
// updateLevel() does NOT call repaint()
// Editor timer calls repaint() once per tick
// Eliminates the 120+ repaints/sec glitch
//==============================================================================
class LEDLadderMeter : public juce::Component
{
public:
    // Renamed from setLevel
    // Does NOT call repaint() - editor timer does that
    void updateLevel(float newLevel)
    {
        if (newLevel > level)
            level = newLevel;
        else
            level = level * 0.92f + newLevel * 0.08f;

        if (newLevel > peak)
        {
            peak     = newLevel;
            peakHold = 90;
        }
        else if (peakHold > 0)
            peakHold--;
        else
            peak *= 0.97f;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float w = bounds.getWidth();
        float h = bounds.getHeight();

        // Housing
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colour(0xFF000000)
            .withAlpha(0.5f));
        g.drawRoundedRectangle(
            bounds.reduced(0.5f), 3.0f, 1.0f);

        float levelDb = juce::Decibels::gainToDecibels(
            level, -60.0f);
        float peakDb  = juce::Decibels::gainToDecibels(
            peak, -60.0f);

        const float ledDb[] = {
            -54.0f, -48.0f, -42.0f, -36.0f, -30.0f,
            -24.0f, -18.0f, -15.0f, -12.0f,  -9.0f,
             -6.0f,  -3.0f,   0.0f,   3.0f };
        const int numLeds = 14;

        float ledH   = (h - 6.0f) / numLeds;
        float ledW   = w - 6.0f;
        float ledGap = 1.5f;

        for (int i = 0; i < numLeds; i++)
        {
            int   idx    = numLeds - 1 - i;
            float ly     = 3.0f + i * ledH;
            bool  active = levelDb >= ledDb[idx];
            bool  isPeak = std::abs(peakDb
                - ledDb[idx]) < 3.5f
                && peakHold > 0
                && !active;

            juce::Colour ledOn, ledOff;

            if (ledDb[idx] >= 3.0f)
            {
                ledOn  = juce::Colour(0xFFC83020);
                ledOff = juce::Colour(0xFF180808);
            }
            else if (ledDb[idx] >= -3.0f)
            {
                ledOn  = juce::Colour(0xFFE84030);
                ledOff = juce::Colour(0xFF180808);
            }
            else if (ledDb[idx] >= -9.0f)
            {
                ledOn  = juce::Colour(0xFFB8A020);
                ledOff = juce::Colour(0xFF141408);
            }
            else
            {
                ledOn  = juce::Colour(0xFF3A8A3A);
                ledOff = juce::Colour(0xFF081408);
            }

            auto ledRect = juce::Rectangle<float>(
                3.0f, ly, ledW, ledH - ledGap);

            if (active)
            {
                // Glow halo
                g.setColour(ledOn.withAlpha(0.12f));
                g.fillRoundedRectangle(
                    ledRect.expanded(1.5f), 2.0f);

                // Body gradient
                juce::ColourGradient ledGrad(
                    ledOn.brighter(0.2f),
                    ledRect.getX(), ledRect.getY(),
                    ledOn.darker(0.3f),
                    ledRect.getX(),
                    ledRect.getBottom(), false);
                g.setGradientFill(ledGrad);
                g.fillRoundedRectangle(ledRect, 1.2f);

                // Dome highlight
                juce::ColourGradient dome(
                    juce::Colours::white
                        .withAlpha(0.18f),
                    ledRect.getX() + ledW * 0.2f,
                    ledRect.getY(),
                    juce::Colours::transparentWhite,
                    ledRect.getX() + ledW * 0.5f,
                    ledRect.getBottom(), false);
                g.setGradientFill(dome);
                g.fillRoundedRectangle(ledRect, 1.2f);
            }
            else
            {
                // Dim LED
                g.setColour(ledOff);
                g.fillRoundedRectangle(ledRect, 1.2f);
            }

            // Peak hold indicator
            if (isPeak)
            {
                g.setColour(ledOn.withAlpha(0.65f));
                g.fillRoundedRectangle(ledRect, 1.2f);
            }
        }

        // Housing highlight
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
        g.setColour(juce::Colour(0xFF252525));
        g.drawHorizontalLine(
            static_cast<int>(bounds.getY()) + 1,
            bounds.getX() + 2,
            bounds.getRight() - 2);
    }

private:
    float level    = 0.0f;
    float peak     = 0.0f;
    int   peakHold = 0;
};

//==============================================================================
// ANALOG LOOK AND FEEL
// Custom rendering for all JUCE controls
//==============================================================================
class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId,
            juce::Colour(0xFFE8E0D0));
        setColour(juce::Slider::textBoxBackgroundColourId,
            juce::Colour(0x00000000));
        setColour(juce::Slider::textBoxOutlineColourId,
            juce::Colour(0x00000000));

        setColour(juce::ComboBox::backgroundColourId,
            juce::Colour(0xFF111111));
        setColour(juce::ComboBox::textColourId,
            juce::Colour(0xFFE8A838));
        setColour(juce::ComboBox::outlineColourId,
            juce::Colour(0xFF2A2A2A));
        setColour(juce::ComboBox::arrowColourId,
            juce::Colour(0xFFE8A838));

        setColour(juce::PopupMenu::backgroundColourId,
            juce::Colour(0xFF0F0F0F));
        setColour(juce::PopupMenu::textColourId,
            juce::Colour(0xFFD8D0C0));
        setColour(
            juce::PopupMenu::highlightedBackgroundColourId,
            juce::Colour(0xFF1A1A10));
        setColour(
            juce::PopupMenu::highlightedTextColourId,
            juce::Colour(0xFFE8A838));

        setColour(juce::Label::textColourId,
            juce::Colour(0xFFD8D0C0));
    }

    //──────────────────────────────────────────────
    // ROTARY KNOB
    // Reads knobStyle property to select filmstrip
    // Falls back to programmatic rendering if
    // TextureCache not yet available
    //──────────────────────────────────────────────
    void drawRotarySlider(
        juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(
            x, y, width, height)
            .toFloat().reduced(4);

        float radius  = juce::jmin(
            bounds.getWidth(),
            bounds.getHeight()) * 0.5f;
        float centreX = bounds.getCentreX();
        float centreY = bounds.getCentreY();
        float angle   = rotaryStartAngle
            + sliderPos
            * (rotaryEndAngle - rotaryStartAngle);

        bool  enabled = slider.isEnabled();
        float alpha   = enabled ? 1.0f : 0.25f;

        // Read style property from slider
        int styleInt = static_cast<int>(
            slider.getProperties()
            .getWithDefault("knobStyle", 0));

        // Try filmstrip rendering first
        auto* tex = TextureCache::getInstance();
        bool drawnFromStrip = false;

        if (tex != nullptr)
        {
            juce::Image* strip = nullptr;

            switch (styleInt)
            {
                case 1:
                    strip = &tex->knobFilmstripVintage;
                    break;
                case 2:
                    strip = &tex->knobFilmstripSSL;
                    break;
                case 3:
                    strip = &tex->knobFilmstripAPI;
                    break;
                default:
                    strip = &tex->knobFilmstrip;
                    break;
            }

            if (strip != nullptr
             && !strip->isNull()
             && strip->getWidth() > 0)
            {
                int numFrames = strip->getHeight()
                    / strip->getWidth();
                int frame = juce::jlimit(0,
                    numFrames - 1,
                    static_cast<int>(
                        sliderPos * (numFrames - 1)));
                int frameH = strip->getWidth();
                int frameY = frame * frameH;
                int kSize  = static_cast<int>(
                    radius * 2.0f);
                int kX = static_cast<int>(
                    centreX - radius);
                int kY = static_cast<int>(
                    centreY - radius);

                g.setOpacity(alpha);
                g.drawImage(*strip,
                    kX, kY, kSize, kSize,
                    0, frameY, frameH, frameH,
                    false);
                g.setOpacity(1.0f);

                drawnFromStrip = true;
            }
        }

        // Fallback programmatic rendering
        // Used if texture not loaded
        if (!drawnFromStrip)
        {
            // Outer shadow
            g.setColour(juce::Colour(0x25000000));
            g.fillEllipse(
                centreX - radius + 1,
                centreY - radius + 1,
                radius * 2, radius * 2);

            // Knob body gradient
            juce::ColourGradient bodyGrad(
                juce::Colour(0xFF3A3A3A)
                    .withAlpha(alpha),
                centreX, centreY - radius,
                juce::Colour(0xFF181818)
                    .withAlpha(alpha),
                centreX, centreY + radius,
                false);
            g.setGradientFill(bodyGrad);
            g.fillEllipse(
                centreX - radius,
                centreY - radius,
                radius * 2, radius * 2);

            // Edge ring
            g.setColour(juce::Colour(0xFF444444)
                .withAlpha(alpha));
            g.drawEllipse(
                centreX - radius,
                centreY - radius,
                radius * 2, radius * 2,
                1.2f);

            // Indicator line
            if (enabled)
            {
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                float lineEnd = radius * 0.65f;

                g.setColour(
                    juce::Colour(0xFFE8A838));
                g.drawLine(
                    centreX, centreY,
                    centreX + lineEnd * cosA,
                    centreY + lineEnd * sinA,
                    2.0f);
            }

            // Center dot
            g.setColour(juce::Colour(0xFF444444)
                .withAlpha(alpha));
            g.fillEllipse(
                centreX - 2.5f,
                centreY - 2.5f,
                5.0f, 5.0f);
        }

        // Track arc - always drawn over the knob
        juce::Path trackArc;
        trackArc.addCentredArc(
            centreX, centreY,
            radius * 0.9f, radius * 0.9f,
            0.0f,
            rotaryStartAngle, rotaryEndAngle,
            true);
        g.setColour(juce::Colour(0xFF1A1A1A)
            .withAlpha(alpha));
        g.strokePath(trackArc,
            juce::PathStrokeType(2.5f));

        // Active arc
        if (sliderPos > 0.005f && enabled)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(
                centreX, centreY,
                radius * 0.9f, radius * 0.9f,
                0.0f,
                rotaryStartAngle, angle,
                true);
            g.setColour(juce::Colour(0xFFE8A838)
                .withAlpha(0.80f));
            g.strokePath(activeArc,
                juce::PathStrokeType(3.0f,
                    juce::PathStrokeType::curved,
                    juce::PathStrokeType::rounded));
        }
    }

    //──────────────────────────────────────────────
    // COMBO BOX
    // No texture blit - removed per audit
    // Texture at 15% opacity was invisible
    // and wasted paint time
    //──────────────────────────────────────────────
    void drawComboBox(
        juce::Graphics& g,
        int width, int height,
        bool isDown,
        int, int, int, int,
        juce::ComboBox&) override
    {
        auto b = juce::Rectangle<float>(
            0, 0,
            static_cast<float>(width),
            static_cast<float>(height));

        // Background
        g.setColour(juce::Colour(
            isDown ? 0xFF181818 : 0xFF111111));
        g.fillRoundedRectangle(b, 3.0f);

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(
            b.reduced(0.5f), 3.0f, 1.0f);

        // Top catch light
        g.setColour(juce::Colour(0xFF1E1E1E));
        g.drawHorizontalLine(1, 2.0f,
            static_cast<float>(width) - 2.0f);

        // Arrow
        float arrowX = static_cast<float>(width)
            - 16.0f;
        float arrowY = static_cast<float>(height)
            * 0.5f;

        juce::Path arrow;
        arrow.addTriangle(
            arrowX,       arrowY - 3.0f,
            arrowX + 7.0f, arrowY,
            arrowX,       arrowY + 3.0f);
        g.setColour(juce::Colour(0xFFE8A838));
        g.fillPath(arrow);
    }

    //──────────────────────────────────────────────
    // TOGGLE BUTTON
    // Fixed bypass LED logic
    // isBypass property inverts glow behavior
    // toggleState=false + isBypass = section ON = glow
    // toggleState=true  + isBypass = bypassed = dark
    //──────────────────────────────────────────────
    void drawToggleButton(
        juce::Graphics& g,
        juce::ToggleButton& button,
        bool highlighted, bool) override
    {
        auto b = button.getLocalBounds()
            .toFloat().reduced(1.0f);

        // Bypass buttons have inverted semantics
        bool isBypass = static_cast<bool>(
            button.getProperties()
            .getWithDefault("isBypass", false));

        bool on = isBypass
            ? !button.getToggleState()
            :  button.getToggleState();

        // Background
        g.setColour(on
            ? juce::Colour(0xFF1A1200)
            : juce::Colour(0xFF0F0F0F));
        g.fillRoundedRectangle(b, 3.0f);

        // Border
        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF222222));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);

        // LED indicator dot
        float dotX = b.getX() + 8.0f;
        float dotY = b.getCentreY();
        float dotR = 2.5f;

        if (on)
        {
            // Glow halo
            g.setColour(juce::Colour(0x28E8A838));
            g.fillEllipse(
                dotX - 5.0f, dotY - 5.0f,
                10.0f, 10.0f);
            // LED body
            g.setColour(juce::Colour(0xFFE8A838));
        }
        else
        {
            g.setColour(juce::Colour(0xFF252525));
        }

        g.fillEllipse(
            dotX - dotR, dotY - dotR,
            dotR * 2.0f, dotR * 2.0f);

        // Button text
        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF3A3A3A));
        g.setFont(juce::Font(
            juce::FontOptions(8.0f)
            .withStyle("Bold")));
        g.drawText(button.getButtonText(),
            b.withLeft(b.getX() + 16.0f),
            juce::Justification::centredLeft);

        // Hover highlight
        if (highlighted)
        {
            g.setColour(
                juce::Colours::white
                .withAlpha(0.02f));
            g.fillRoundedRectangle(b, 3.0f);
        }
    }

    //──────────────────────────────────────────────
    // LABEL
    //──────────────────────────────────────────────
    void drawLabel(
        juce::Graphics& g,
        juce::Label& label) override
    {
        g.setColour(label.findColour(
            juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawText(label.getText(),
            label.getLocalBounds(),
            label.getJustificationType());
    }
};