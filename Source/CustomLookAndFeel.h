#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// ANALOG NEEDLE METER
// Physics-based VU meter with inertia and damping
// Model-specific face plate colors
//==============================================================================
class AnalogNeedleMeter : public juce::Component,
                          public juce::Timer
{
public:
    AnalogNeedleMeter()
    {
        startTimerHz(60);
    }

    ~AnalogNeedleMeter() override
    {
        stopTimer();
    }

    void setGainReduction(float grDb)
    {
        targetAngle = juce::jlimit(0.0f, 1.0f,
            grDb / 20.0f);
    }

    // Model specific face plate
    // 0=SSL 1=Fairchild 2=LA2A 3=1176 4=API
    void setModel(int m)
    {
        model = m;
        repaint();
    }

    void timerCallback() override
    {
        // Needle physics - mass and damping
        float force = (targetAngle - needleAngle) * 8.0f;
        needleVelocity += force;
        needleVelocity *= 0.65f; // damping
        needleAngle += needleVelocity * 0.016f;
        needleAngle = juce::jlimit(0.0f, 1.0f,
            needleAngle);

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float cx = w * 0.5f;
        float pivotY = h * 0.85f;

        // Face plate background
        juce::Colour faceColor;
        juce::Colour textColor;
        juce::Colour needleColor;
        juce::Colour glowColor;

        switch (model)
        {
            case 0: // SSL
                faceColor   = juce::Colour(0xFFF0F0F0);
                textColor   = juce::Colour(0xFF1A1A1A);
                needleColor = juce::Colour(0xFF1A1A1A);
                glowColor   = juce::Colour(0x2040C040);
                break;
            case 1: // Fairchild
                faceColor   = juce::Colour(0xFFE8D8C0);
                textColor   = juce::Colour(0xFF2A1A0A);
                needleColor = juce::Colour(0xFF1A1A1A);
                glowColor   = juce::Colour(0x20C8A838);
                break;
            case 2: // LA-2A
                faceColor   = juce::Colour(0xFFE8E0D8);
                textColor   = juce::Colour(0xFF2A2A2A);
                needleColor = juce::Colour(0xFF1A1A1A);
                glowColor   = juce::Colour(0x20808060);
                break;
            case 3: // 1176
                faceColor   = juce::Colour(0xFF1A1A1A);
                textColor   = juce::Colour(0xFFD0D0D0);
                needleColor = juce::Colour(0xFFD0D0D0);
                glowColor   = juce::Colour(0x20606060);
                break;
            case 4: // API
                faceColor   = juce::Colour(0xFFF0F0F0);
                textColor   = juce::Colour(0xFF1A2A3A);
                needleColor = juce::Colour(0xFF1A1A1A);
                glowColor   = juce::Colour(0x204488FF);
                break;
            default:
                faceColor   = juce::Colour(0xFFE8E0D8);
                textColor   = juce::Colour(0xFF2A2A2A);
                needleColor = juce::Colour(0xFF1A1A1A);
                glowColor   = juce::Colour(0x20808060);
                break;
        }

        // Outer bezel
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Inner bezel
        g.setColour(juce::Colour(0xFF3A3A3A));
        g.fillRoundedRectangle(
            bounds.reduced(2.0f), 3.0f);

        // Face plate
        auto faceRect = bounds.reduced(4.0f);
        g.setColour(faceColor);
        g.fillRoundedRectangle(faceRect, 2.0f);

        // Subtle backlight glow
        g.setColour(glowColor);
        g.fillRoundedRectangle(faceRect, 2.0f);

        // Scale arc
        float arcRadius = w * 0.35f;
        float startAngle = -2.4f;
        float endAngle   =  0.4f;

        // Draw scale markings
        g.setColour(textColor);
        g.setFont(juce::Font(
            juce::FontOptions(8.0f)));

        const float markAngles[] =
            { 0.0f, 0.15f, 0.3f, 0.45f,
              0.6f, 0.75f, 0.9f, 1.0f };
        const char* markLabels[] =
            { "0", "-3", "-5", "-7",
              "-10", "-15", "-20", "" };

        for (int m = 0; m < 8; m++)
        {
            float angle = startAngle
                + markAngles[m]
                * (endAngle - startAngle);

            float mx = cx + arcRadius
                     * std::cos(angle);
            float my = pivotY + arcRadius
                     * std::sin(angle) - h * 0.15f;

            // Tick mark
            float innerR = arcRadius * 0.85f;
            float ix = cx + innerR * std::cos(angle);
            float iy = pivotY + innerR
                     * std::sin(angle) - h * 0.15f;

            g.setColour(textColor);
            g.drawLine(ix, iy, mx, my, 1.0f);

            // Label
            if (markLabels[m][0] != '\0')
            {
                g.drawText(markLabels[m],
                    static_cast<int>(mx - 12),
                    static_cast<int>(my - 14),
                    24, 12,
                    juce::Justification::centred);
            }
        }

        // Red zone above -3dB
        float redStart = startAngle
            + 0.15f * (endAngle - startAngle);
        g.setColour(juce::Colour(0x40C83030));

        juce::Path redArc;
        redArc.addCentredArc(cx,
            pivotY - h * 0.15f,
            arcRadius * 0.9f,
            arcRadius * 0.9f,
            0.0f,
            redStart, endAngle,
            true);
        g.strokePath(redArc,
            juce::PathStrokeType(3.0f));

        // Needle
        float needleRotation = startAngle
            + needleAngle * (endAngle - startAngle);

        float needleLength = arcRadius * 1.1f;
        float nx = cx + needleLength
                 * std::cos(needleRotation);
        float ny = pivotY + needleLength
                 * std::sin(needleRotation)
                 - h * 0.15f;

        // Needle shadow
        g.setColour(juce::Colour(0x30000000));
        g.drawLine(cx + 1, pivotY - h * 0.15f + 1,
                   nx + 1, ny + 1, 2.0f);

        // Needle body
        g.setColour(needleColor);
        g.drawLine(cx, pivotY - h * 0.15f,
                   nx, ny, 1.5f);

        // Needle pivot dot
        g.setColour(needleColor);
        g.fillEllipse(cx - 3, pivotY - h * 0.15f - 3,
                      6, 6);

        // Label
        g.setColour(textColor.withAlpha(0.6f));
        g.setFont(juce::Font(
            juce::FontOptions(7.0f)));
        g.drawText("GAIN REDUCTION  dB",
            0, static_cast<int>(h * 0.7f),
            static_cast<int>(w), 12,
            juce::Justification::centred);

        // Glass reflection overlay
        g.setGradientFill(
            juce::ColourGradient(
                juce::Colour(0x15FFFFFF),
                0, 0,
                juce::Colour(0x00FFFFFF),
                0, h * 0.5f,
                false));
        g.fillRoundedRectangle(faceRect, 2.0f);
    }

private:
    float targetAngle    = 0.0f;
    float needleAngle    = 0.0f;
    float needleVelocity = 0.0f;
    int   model          = 0;
};

//==============================================================================
// LED LADDER METER
// Realistic LED segments with glow and phosphor decay
//==============================================================================
class LEDLadderMeter : public juce::Component
{
public:
    void setLevel(float newLevel)
    {
        // Peak with fast attack slow release
        if (newLevel > level)
            level = newLevel;
        else
            level = level * 0.92f + newLevel * 0.08f;

        // Peak hold
        if (newLevel > peak)
        {
            peak     = newLevel;
            peakHold = 90;
        }
        else if (peakHold > 0)
        {
            peakHold--;
        }
        else
        {
            peak *= 0.97f;
        }

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float w = bounds.getWidth();
        float h = bounds.getHeight();

        // Dark background
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Panel surround
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        float levelDb = juce::Decibels::gainToDecibels(
            level, -60.0f);
        float peakDb  = juce::Decibels::gainToDecibels(
            peak, -60.0f);

        // LED positions (dB thresholds)
        const float ledDb[] =
            { -48, -42, -36, -30, -24,
              -18, -15, -12, -9, -6,
              -3, 0, 3, 6 };
        const int numLeds = 14;

        float ledH    = (h - 8.0f) / numLeds;
        float ledW    = w - 8.0f;
        float ledGap  = 2.0f;

        for (int i = 0; i < numLeds; i++)
        {
            // LED position - bottom to top
            int ledIndex = numLeds - 1 - i;
            float ly = 4.0f + i * ledH;

            // LED color based on level
            juce::Colour ledOff;
            juce::Colour ledOn;

            if (ledDb[ledIndex] >= 3)
            {
                ledOff = juce::Colour(0xFF1A0808);
                ledOn  = juce::Colour(0xFFC83020);
            }
            else if (ledDb[ledIndex] >= 0)
            {
                ledOff = juce::Colour(0xFF1A0808);
                ledOn  = juce::Colour(0xFFE84030);
            }
            else if (ledDb[ledIndex] >= -6)
            {
                ledOff = juce::Colour(0xFF181808);
                ledOn  = juce::Colour(0xFFC8A030);
            }
            else
            {
                ledOff = juce::Colour(0xFF081808);
                ledOn  = juce::Colour(0xFF3A8A3A);
            }

            // Is this LED active?
            bool active = levelDb >= ledDb[ledIndex];
            bool isPeak = std::abs(peakDb
                - ledDb[ledIndex]) < 3.0f
                && peakHold > 0;

            juce::Colour ledColor = active
                ? ledOn : ledOff;

            // LED body
            auto ledRect = juce::Rectangle<float>(
                4.0f, ly, ledW, ledH - ledGap);
            g.setColour(ledColor);
            g.fillRoundedRectangle(ledRect, 1.5f);

            // Glow effect when active
            if (active)
            {
                g.setColour(ledOn.withAlpha(0.15f));
                g.fillRoundedRectangle(
                    ledRect.expanded(2.0f), 2.5f);
            }

            // Peak hold indicator
            if (isPeak && !active)
            {
                g.setColour(ledOn.withAlpha(0.7f));
                g.fillRoundedRectangle(ledRect, 1.5f);
            }

            // Subtle 3D dome effect
            if (active)
            {
                g.setGradientFill(
                    juce::ColourGradient(
                        juce::Colours::white
                            .withAlpha(0.15f),
                        ledRect.getX(),
                        ledRect.getY(),
                        juce::Colours::white
                            .withAlpha(0.0f),
                        ledRect.getX(),
                        ledRect.getBottom(),
                        false));
                g.fillRoundedRectangle(ledRect, 1.5f);
            }
        }
    }

private:
    float level    = 0.0f;
    float peak     = 0.0f;
    int   peakHold = 0;
};

//==============================================================================
// HARDWARE SCREW
// Decorative detail
//==============================================================================
class HardwareScrew : public juce::Component
{
public:
    void setRotation(float r)
    {
        rotation = r;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(1);
        float cx = b.getCentreX();
        float cy = b.getCentreY();
        float r  = b.getWidth() * 0.5f;

        // Outer ring
        g.setColour(juce::Colour(0xFF4A4A4A));
        g.fillEllipse(b);

        // Inner body
        g.setColour(juce::Colour(0xFF5A5A5A));
        g.fillEllipse(b.reduced(1.5f));

        // Phillips cross
        g.setColour(juce::Colour(0xFF3A3A3A));

        float slotLen = r * 0.7f;
        float angle1  = rotation;
        float angle2  = rotation
                      + juce::MathConstants<float>::pi
                      * 0.5f;

        g.drawLine(
            cx + slotLen * std::cos(angle1),
            cy + slotLen * std::sin(angle1),
            cx - slotLen * std::cos(angle1),
            cy - slotLen * std::sin(angle1),
            1.2f);
        g.drawLine(
            cx + slotLen * std::cos(angle2),
            cy + slotLen * std::sin(angle2),
            cx - slotLen * std::cos(angle2),
            cy - slotLen * std::sin(angle2),
            1.2f);

        // Highlight
        g.setColour(juce::Colour(0x15FFFFFF));
        g.fillEllipse(
            cx - r * 0.3f, cy - r * 0.4f,
            r * 0.6f, r * 0.4f);
    }

private:
    float rotation = 0.3f;
};

//==============================================================================
// CUSTOM LOOK AND FEEL
// Transforms all JUCE controls to analog hardware style
//==============================================================================
class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel()
    {
        // Global colors
        setColour(juce::Slider::textBoxTextColourId,
            juce::Colour(0xFFE8C878));
        setColour(juce::Slider::textBoxBackgroundColourId,
            juce::Colour(0xFF0A0A0A));
        setColour(juce::Slider::textBoxOutlineColourId,
            juce::Colour(0xFF0A0A0A));

        setColour(juce::ComboBox::backgroundColourId,
            juce::Colour(0xFF1A1A1A));
        setColour(juce::ComboBox::textColourId,
            juce::Colour(0xFFE8A838));
        setColour(juce::ComboBox::outlineColourId,
            juce::Colour(0xFF2A2A2A));
        setColour(juce::ComboBox::arrowColourId,
            juce::Colour(0xFFE8A838));

        setColour(juce::PopupMenu::backgroundColourId,
            juce::Colour(0xFF1A1A1A));
        setColour(juce::PopupMenu::textColourId,
            juce::Colour(0xFFE8C878));
        setColour(
            juce::PopupMenu::highlightedBackgroundColourId,
            juce::Colour(0xFF2A2A1A));
        setColour(juce::PopupMenu::highlightedTextColourId,
            juce::Colour(0xFFE8A838));

        setColour(juce::Label::textColourId,
            juce::Colour(0xFFE8C878));
    }

    //──────────────────────────────────────────
    // ROTARY KNOB DRAWING
    // Dark metal body with amber indicator arc
    //──────────────────────────────────────────
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(
            x, y, width, height).toFloat().reduced(4);

        float radius  = juce::jmin(bounds.getWidth(),
            bounds.getHeight()) * 0.5f;
        float centreX = bounds.getCentreX();
        float centreY = bounds.getCentreY();

        float angle = rotaryStartAngle
            + sliderPos
            * (rotaryEndAngle - rotaryStartAngle);

        bool enabled = slider.isEnabled();
        float alpha  = enabled ? 1.0f : 0.3f;

        // Outer shadow
        g.setColour(juce::Colour(0xFF000000)
            .withAlpha(0.3f * alpha));
        g.fillEllipse(centreX - radius + 1,
                      centreY - radius + 1,
                      radius * 2, radius * 2);

        // Track arc (background)
        juce::Path trackArc;
        trackArc.addCentredArc(centreX, centreY,
            radius * 0.85f, radius * 0.85f,
            0.0f,
            rotaryStartAngle, rotaryEndAngle,
            true);
        g.setColour(juce::Colour(0xFF1A1A1A)
            .withAlpha(alpha));
        g.strokePath(trackArc,
            juce::PathStrokeType(3.0f));

        // Active arc (amber)
        if (sliderPos > 0.01f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY,
                radius * 0.85f, radius * 0.85f,
                0.0f,
                rotaryStartAngle, angle,
                true);
            g.setColour(juce::Colour(0xFFE8A838)
                .withAlpha(0.7f * alpha));
            g.strokePath(activeArc,
                juce::PathStrokeType(3.0f));
        }

        // Knob body - dark metal gradient
        juce::ColourGradient bodyGrad(
            juce::Colour(0xFF3A3A3A).withAlpha(alpha),
            centreX, centreY - radius,
            juce::Colour(0xFF1A1A1A).withAlpha(alpha),
            centreX, centreY + radius,
            false);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(centreX - radius * 0.7f,
                      centreY - radius * 0.7f,
                      radius * 1.4f,
                      radius * 1.4f);

        // Knob edge ring
        g.setColour(juce::Colour(0xFF4A4A4A)
            .withAlpha(alpha));
        g.drawEllipse(centreX - radius * 0.7f,
                      centreY - radius * 0.7f,
                      radius * 1.4f,
                      radius * 1.4f,
                      1.5f);

        // Knurled edge texture (subtle dots)
        if (radius > 20)
        {
            g.setColour(juce::Colour(0xFF2A2A2A)
                .withAlpha(0.5f * alpha));
            int numDots = 24;
            float dotR  = radius * 0.75f;
            for (int i = 0; i < numDots; i++)
            {
                float dotAngle = (float)i
                    / numDots * juce::MathConstants<float>::twoPi;
                float dx = centreX
                    + dotR * std::cos(dotAngle);
                float dy = centreY
                    + dotR * std::sin(dotAngle);
                g.fillEllipse(dx - 1, dy - 1, 2, 2);
            }
        }

        // Indicator line
        float lineLen = radius * 0.5f;
        float startR  = radius * 0.2f;
        g.setColour(juce::Colour(0xFFE8A838)
            .withAlpha(alpha));
        g.drawLine(
            centreX + startR * std::cos(angle),
            centreY + startR * std::sin(angle),
            centreX + lineLen * std::cos(angle),
            centreY + lineLen * std::sin(angle),
            2.5f);

        // Center dot
        g.setColour(juce::Colour(0xFF555555)
            .withAlpha(alpha));
        g.fillEllipse(centreX - 3, centreY - 3,
                      6, 6);

        // Top highlight
        g.setColour(juce::Colour(0x0CFFFFFF)
            .withAlpha(alpha));
        g.fillEllipse(
            centreX - radius * 0.3f,
            centreY - radius * 0.5f,
            radius * 0.6f,
            radius * 0.3f);
    }

    //──────────────────────────────────────────
    // COMBO BOX DRAWING
    // Dark panel with amber text
    //──────────────────────────────────────────
    void drawComboBox(juce::Graphics& g,
        int width, int height, bool isDown,
        int, int, int, int,
        juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(
            0, 0, width, height).toFloat();

        // Background
        g.setColour(juce::Colour(
            isDown ? 0xFF222222 : 0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Border
        g.setColour(juce::Colour(0xFF3A3A3A));
        g.drawRoundedRectangle(
            bounds.reduced(0.5f), 3.0f, 1.0f);

        // Arrow
        float arrowX = width - 18.0f;
        float arrowY = height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle(
            arrowX, arrowY - 3,
            arrowX + 8, arrowY - 3,
            arrowX + 4, arrowY + 3);
        g.setColour(juce::Colour(0xFFE8A838));
        g.fillPath(arrow);
    }

    //──────────────────────────────────────────
    // TOGGLE BUTTON DRAWING
    // Hardware style illuminated button
    //──────────────────────────────────────────
    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& button,
        bool highlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds()
            .toFloat().reduced(1);

        bool on = button.getToggleState();

        // Background
        g.setColour(on
            ? juce::Colour(0xFF2A1A00)
            : juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Border
        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF333333));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // LED indicator dot
        float dotX = bounds.getX() + 8;
        float dotY = bounds.getCentreY();
        float dotR = 3.0f;

        if (on)
        {
            // LED glow
            g.setColour(juce::Colour(0x30E8A838));
            g.fillEllipse(dotX - 5, dotY - 5, 10, 10);
            // LED body
            g.setColour(juce::Colour(0xFFE8A838));
            g.fillEllipse(dotX - dotR, dotY - dotR,
                         dotR * 2, dotR * 2);
        }
        else
        {
            g.setColour(juce::Colour(0xFF333333));
            g.fillEllipse(dotX - dotR, dotY - dotR,
                         dotR * 2, dotR * 2);
        }

        // Text
        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF555555));
        g.setFont(juce::Font(
            juce::FontOptions(9.0f).withStyle("Bold")));
        g.drawText(button.getButtonText(),
            bounds.withLeft(bounds.getX() + 16),
            juce::Justification::centredLeft);

        // Hover highlight
        if (highlighted)
        {
            g.setColour(
                juce::Colours::white.withAlpha(0.03f));
            g.fillRoundedRectangle(bounds, 3.0f);
        }
    }

    //──────────────────────────────────────────
    // LABEL DRAWING
    // Clean amber text
    //──────────────────────────────────────────
    void drawLabel(juce::Graphics& g,
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

//==============================================================================
// BRUSHED METAL BACKGROUND
// Renders a realistic brushed metal texture
//==============================================================================
namespace PanelTextures
{
    inline void drawBrushedMetal(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        juce::Colour baseColor,
        float scratchIntensity = 0.03f)
    {
        // Base color
        g.setColour(baseColor);
        g.fillRoundedRectangle(area, 5.0f);

        // Vertical gradient for depth
        g.setGradientFill(
            juce::ColourGradient(
                juce::Colours::white.withAlpha(0.04f),
                area.getX(), area.getY(),
                juce::Colours::black.withAlpha(0.06f),
                area.getX(), area.getBottom(),
                false));
        g.fillRoundedRectangle(area, 5.0f);

        // Horizontal brush lines
        juce::Random rng(42);
        g.setColour(juce::Colours::white
            .withAlpha(scratchIntensity));

        for (int i = 0; i < 60; i++)
        {
            float y = area.getY()
                + rng.nextFloat() * area.getHeight();
            float x1 = area.getX()
                + rng.nextFloat() * area.getWidth()
                * 0.3f;
            float x2 = x1 + rng.nextFloat()
                * area.getWidth() * 0.7f;
            g.drawLine(x1, y, x2, y, 0.5f);
        }

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(area, 5.0f, 1.0f);
    }

    inline void drawDust(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        float intensity = 0.02f)
    {
        juce::Random rng(123);
        g.setColour(juce::Colours::white
            .withAlpha(intensity));

        for (int i = 0; i < 30; i++)
        {
            float x = area.getX()
                + rng.nextFloat() * area.getWidth();
            float y = area.getY()
                + rng.nextFloat() * area.getHeight();
            float s = 0.5f + rng.nextFloat() * 1.5f;
            g.fillEllipse(x, y, s, s);
        }
    }
}