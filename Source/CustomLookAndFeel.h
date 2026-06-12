#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <array>
#include "TextureGenerator.h"

//==============================================================================
// KNOB VISUAL STYLE
//==============================================================================
enum class KnobVisualStyle
{
    DarkMetal   = 0,
    VintageDome = 1,
    SSLCompact  = 2,
    APISkirt    = 3
};

inline void setKnobVisualStyle(
    juce::Slider& slider, KnobVisualStyle style)
{
    slider.getProperties().set("knobStyle",
        static_cast<int>(style));
}

//==============================================================================
// PANEL TEXTURES
//==============================================================================
namespace PanelTextures
{
    inline void drawBrushedMetal(
        juce::Graphics& g,
        juce::Rectangle<float> area,
        juce::Colour tintColor,
        float scratchIntensity = 0.03f)
    {
        g.setColour(tintColor);
        g.fillRoundedRectangle(area, 4.0f);

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

            // Glass reflection overlay
            if (!tex->glassReflection.isNull())
            {
                g.setOpacity(0.04f);
                g.drawImage(tex->glassReflection,
                    static_cast<int>(area.getX()),
                    static_cast<int>(area.getY()),
                    static_cast<int>(area.getWidth()),
                    static_cast<int>(area.getHeight()),
                    0, 0,
                    tex->glassReflection.getWidth(),
                    tex->glassReflection.getHeight());
                g.setOpacity(1.0f);
            }
        }

        g.setColour(tintColor.brighter(0.08f));
        g.drawHorizontalLine(
            static_cast<int>(area.getY()) + 1,
            area.getX() + 3, area.getRight() - 3);

        g.setColour(tintColor.darker(0.15f));
        g.drawHorizontalLine(
            static_cast<int>(area.getBottom()) - 2,
            area.getX() + 3, area.getRight() - 3);

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
        if (tex != nullptr && !tex->wrinkleFinish.isNull())
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
        if (tex == nullptr || tex->dustOverlay.isNull())
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
        if (tex == nullptr || tex->screwHead.isNull())
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
            s, s, 0, 0,
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
            juce::FontOptions(10.0f).withStyle("Bold")));
        g.setColour(juce::Colour(0xFF0A0A0A));
        g.drawText(text, bounds.translated(1, 1), just);
        g.setColour(juce::Colour(0xFFE8C878));
        g.drawText(text, bounds, just);
    }

    inline void drawWoodPanel(
        juce::Graphics& g,
        juce::Rectangle<float> area)
    {
        auto* tex = TextureCache::getInstance();
        if (tex == nullptr || tex->woodGrain.isNull())
        {
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
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.05f),
            area.getX(), area.getY(),
            juce::Colours::transparentWhite,
            area.getRight(), area.getY(), false));
        g.fillRect(area);
    }
}

//==============================================================================
// EQ FREQUENCY RESPONSE DISPLAY
// Shows combined filter curve above EQ knobs
// Updated only when parameters change
//==============================================================================
class EQCurveDisplay : public juce::Component
{
public:
    EQCurveDisplay()
    {
        magnitudes.resize(numPoints, 1.0);
    }

    // Call when EQ parameters change
    void updateCurve(
        double sampleRate,
        float lowGain, float lowFreq,
        float midGain, float midFreq, float midQ,
        float highGain, float highFreq)
    {
        sr = sampleRate;

        // Evaluate at numPoints log-spaced frequencies
        for (int i = 0; i < numPoints; i++)
        {
            double freq = 20.0 * std::pow(
                1000.0, static_cast<double>(i)
                / (numPoints - 1));

            double w = 2.0
                * juce::MathConstants<double>::pi
                * freq / sr;

            double mag = 1.0;

            // Low shelf contribution
            if (std::abs(lowGain) > 0.1f)
                mag *= evalShelf(w, lowFreq, lowGain, true);

            // Mid peak contribution
            if (std::abs(midGain) > 0.1f)
                mag *= evalPeak(w, midFreq, midGain, midQ);

            // High shelf contribution
            if (std::abs(highGain) > 0.1f)
                mag *= evalShelf(w, highFreq, highGain, false);

            magnitudes[i] = mag;
        }

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        float w = b.getWidth();
        float h = b.getHeight();

        // Background
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(b, 3.0f);

        // Grid lines at 0dB
        float zeroY = h * 0.5f;
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawHorizontalLine(static_cast<int>(zeroY),
            b.getX(), b.getRight());

        // Reference lines at +/-6dB
        float sixDbPx = h * 0.5f * (6.0f / 15.0f);
        g.setColour(juce::Colour(0xFF141414));
        g.drawHorizontalLine(
            static_cast<int>(zeroY - sixDbPx),
            b.getX(), b.getRight());
        g.drawHorizontalLine(
            static_cast<int>(zeroY + sixDbPx),
            b.getX(), b.getRight());

        // Frequency curve
        juce::Path curvePath;
        bool started = false;

        for (int i = 0; i < numPoints; i++)
        {
            float px = b.getX()
                + static_cast<float>(i)
                / (numPoints - 1) * w;

            // Convert magnitude to dB
            float db = static_cast<float>(
                20.0 * std::log10(
                    std::max(magnitudes[i], 0.001)));

            // Map dB to pixel Y
            // +15dB = top, -15dB = bottom
            float py = zeroY - (db / 15.0f) * zeroY;
            py = juce::jlimit(b.getY(), b.getBottom(), py);

            if (!started)
            {
                curvePath.startNewSubPath(px, py);
                started = true;
            }
            else
            {
                curvePath.lineTo(px, py);
            }
        }

        // Fill below curve
        juce::Path fillPath = curvePath;
        fillPath.lineTo(b.getRight(), zeroY);
        fillPath.lineTo(b.getX(), zeroY);
        fillPath.closeSubPath();

        g.setColour(juce::Colour(0xFFE8A838)
            .withAlpha(0.08f));
        g.fillPath(fillPath);

        // Curve line
        g.setColour(juce::Colour(0xFFE8A838)
            .withAlpha(0.7f));
        g.strokePath(curvePath,
            juce::PathStrokeType(1.5f));

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);
    }

private:
    static constexpr int numPoints = 128;
    std::vector<double> magnitudes;
    double sr = 44100.0;

    // Evaluate low/high shelf magnitude at frequency w
    double evalShelf(double w, float freq,
                     float gainDb, bool isLow)
    {
        double A  = std::pow(10.0, gainDb / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi
                  * freq / sr;
        double cosw = std::cos(w0);
        double sinw = std::sin(w0);
        double S = 0.8;
        double alpha = sinw / 2.0
            * std::sqrt((A + 1.0/A)
            * (1.0/S - 1.0) + 2.0);
        double sqA = std::sqrt(A);

        double b0, b1, b2, a0, a1, a2;

        if (isLow)
        {
            b0 = A*((A+1)-(A-1)*cosw+2*sqA*alpha);
            b1 = 2*A*((A-1)-(A+1)*cosw);
            b2 = A*((A+1)-(A-1)*cosw-2*sqA*alpha);
            a0 = (A+1)+(A-1)*cosw+2*sqA*alpha;
            a1 = -2*((A-1)+(A+1)*cosw);
            a2 = (A+1)+(A-1)*cosw-2*sqA*alpha;
        }
        else
        {
            b0 = A*((A+1)+(A-1)*cosw+2*sqA*alpha);
            b1 = -2*A*((A-1)+(A+1)*cosw);
            b2 = A*((A+1)+(A-1)*cosw-2*sqA*alpha);
            a0 = (A+1)-(A-1)*cosw+2*sqA*alpha;
            a1 = 2*((A-1)-(A+1)*cosw);
            a2 = (A+1)-(A-1)*cosw-2*sqA*alpha;
        }

        // Evaluate H(e^jw) numerically
        // |H| = |B(e^jw)| / |A(e^jw)|
        double cosw_w = std::cos(w);
        double sinw_w = std::sin(w);
        double cos2w  = std::cos(2.0 * w);
        double sin2w  = std::sin(2.0 * w);

        double Bre = b0 + b1*cosw_w + b2*cos2w;
        double Bim = -b1*sinw_w - b2*sin2w;
        double Are = a0 + a1*cosw_w + a2*cos2w;
        double Aim = -a1*sinw_w - a2*sin2w;

        double Bmag = std::sqrt(Bre*Bre + Bim*Bim);
        double Amag = std::sqrt(Are*Are + Aim*Aim);

        return (Amag > 0.0001) ? Bmag / Amag : 1.0;
    }

    // Evaluate peaking EQ magnitude
    double evalPeak(double w, float freq,
                    float gainDb, float Q)
    {
        double A     = std::pow(10.0, gainDb / 40.0);
        double w0    = 2.0 * juce::MathConstants<double>::pi
                     * freq / sr;
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * Q);

        double b0 = 1.0 + alpha * A;
        double b1 = -2.0 * std::cos(w0);
        double b2 = 1.0 - alpha * A;
        double a0 = 1.0 + alpha / A;
        double a1 = -2.0 * std::cos(w0);
        double a2 = 1.0 - alpha / A;

        double cosw_w = std::cos(w);
        double sinw_w = std::sin(w);
        double cos2w  = std::cos(2.0 * w);
        double sin2w  = std::sin(2.0 * w);

        double Bre = b0 + b1*cosw_w + b2*cos2w;
        double Bim = -b1*sinw_w - b2*sin2w;
        double Are = a0 + a1*cosw_w + a2*cos2w;
        double Aim = -a1*sinw_w - a2*sin2w;

        double Bmag = std::sqrt(Bre*Bre + Bim*Bim);
        double Amag = std::sqrt(Are*Are + Aim*Aim);

        return (Amag > 0.0001) ? Bmag / Amag : 1.0;
    }
};

//==============================================================================
// GR HISTORY TRACE
// 2-second ring buffer of gain reduction
// Drawn as waveform behind the needle meter
//==============================================================================
class GRHistoryTrace : public juce::Component
{
public:
    GRHistoryTrace()
    {
        history.fill(0.0f);
    }

    // Call from editor timer at 30Hz
    void pushGR(float grDb)
    {
        // Normalize 0-20dB range to 0-1
        float normalized = juce::jlimit(0.0f, 1.0f,
            grDb / 20.0f);

        history[writePos] = normalized;
        writePos = (writePos + 1) % historySize;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        float w = b.getWidth();
        float h = b.getHeight();

        // Transparent background
        g.setColour(juce::Colour(0x00000000));
        g.fillRect(b);

        if (w < 2) return;

        // Draw history as waveform
        juce::Path tracePath;
        bool started = false;

        for (int i = 0; i < historySize; i++)
        {
            // Read from oldest to newest
            int idx = (writePos + i) % historySize;
            float val = history[idx];

            float px = b.getX()
                + static_cast<float>(i)
                / (historySize - 1) * w;
            float py = b.getBottom()
                - val * h;

            if (!started)
            {
                tracePath.startNewSubPath(px, py);
                started = true;
            }
            else
            {
                tracePath.lineTo(px, py);
            }
        }

        // Fill
        juce::Path fillPath = tracePath;
        fillPath.lineTo(b.getRight(), b.getBottom());
        fillPath.lineTo(b.getX(), b.getBottom());
        fillPath.closeSubPath();

        g.setColour(juce::Colour(0xFF00A8C8)
            .withAlpha(0.12f));
        g.fillPath(fillPath);

        // Line
        g.setColour(juce::Colour(0xFF00A8C8)
            .withAlpha(0.4f));
        g.strokePath(tracePath,
            juce::PathStrokeType(1.0f));
    }

private:
    static constexpr int historySize = 60; // 2s at 30Hz
    std::array<float, 60> history;
    int writePos = 0;
};

//==============================================================================
// CLIP LATCH LED
// Latches when signal exceeds threshold
// Click to clear
//==============================================================================
class ClipLatchLED : public juce::Component
{
public:
    ClipLatchLED()
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    // Call from editor timer
    void checkLevel(float peakLinear)
    {
        if (peakLinear >= 0.944f) // -0.5dBFS
        {
            clipped = true;
            repaint();
        }
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        clipped = false;
        repaint();
    }

    bool isClipped() const { return clipped; }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        if (clipped)
        {
            // Red glow
            g.setColour(juce::Colour(0x40C83020));
            g.fillEllipse(b.expanded(2.0f));
            // Red LED
            g.setColour(juce::Colour(0xFFC83020));
            g.fillEllipse(b);
            // Highlight
            g.setColour(juce::Colours::white
                .withAlpha(0.25f));
            g.fillEllipse(b.reduced(2).withHeight(
                b.getHeight() * 0.4f));
        }
        else
        {
            // Dark LED
            g.setColour(juce::Colour(0xFF1A0808));
            g.fillEllipse(b);
        }

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawEllipse(b, 1.0f);
    }

private:
    bool clipped = false;
};

//==============================================================================
// ANALOG NEEDLE METER
// NO INTERNAL TIMER - editor drives physics
// VU face selected per compressor model
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
        if (m != model) { model = m; repaint(); }
    }

    void advancePhysics()
    {
        float force = (targetAngle - needleAngle) * 0.15f;
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

        // VU face image - per model as per review
        juce::Image* faceImg = nullptr;

        switch (model)
        {
            case 0: // SSL - white face, console style
                faceColor = juce::Colour(0xFFF0F0F0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x2040C040);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceWhite : nullptr;
                break;
            case 1: // Fairchild - cream, vintage tube
                faceColor = juce::Colour(0xFFE8D8C0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20C8A838);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceCream : nullptr;
                break;
            case 2: // LA-2A - warm cream, opto
                faceColor = juce::Colour(0xFFE8E0D0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20D4900A);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceCream : nullptr;
                break;
            case 3: // 1176 - black face, FET
                faceColor = juce::Colour(0xFF0A0A0A);
                needleCol = juce::Colour(0xFFD0D0D0);
                glowCol   = juce::Colour(0x20606060);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceBlack : nullptr;
                break;
            case 4: // API - white, clean modern
                faceColor = juce::Colour(0xFFF0F0F0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x154488FF);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceWhite : nullptr;
                break;
            default:
                faceColor = juce::Colour(0xFFE8E0D0);
                needleCol = juce::Colour(0xFF1A1A1A);
                glowCol   = juce::Colour(0x20808060);
                faceImg   = (tex != nullptr)
                    ? &tex->vuFaceCream : nullptr;
                break;
        }

        // Outer bezel
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(juce::Colour(0xFF303030));
        g.drawRoundedRectangle(bounds, 5.0f, 1.5f);
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 4.0f);

        auto faceRect = bounds.reduced(4.0f);

        // Draw VU face image
        if (faceImg != nullptr && !faceImg->isNull())
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

        // Needle physics
        float startAngle = -2.35f;
        float sweepAngle = 4.7f;
        float needleRot  = startAngle + needleAngle * sweepAngle;
        float needleLen  = h * 0.62f;
        float nx = cx + needleLen * std::cos(needleRot);
        float ny = adjustY + needleLen * std::sin(needleRot);

        // Needle shadow
        g.setColour(juce::Colour(0x18000000));
        g.drawLine(cx+0.8f, adjustY+0.8f,
                   nx+0.8f, ny+0.8f, 2.0f);

        // Tapered needle
        float baseW = 1.8f, tipW = 0.3f;
        float perpA = needleRot
            + juce::MathConstants<float>::pi * 0.5f;
        float pcosA = std::cos(perpA);
        float psinA = std::sin(perpA);

        juce::Path needle;
        needle.startNewSubPath(cx+baseW*pcosA, adjustY+baseW*psinA);
        needle.lineTo(nx+tipW*pcosA, ny+tipW*psinA);
        needle.lineTo(nx-tipW*pcosA, ny-tipW*psinA);
        needle.lineTo(cx-baseW*pcosA, adjustY-baseW*psinA);
        needle.closeSubPath();
        g.setColour(needleCol);
        g.fillPath(needle);

        // Pivot
        float pivR = 5.0f;
        g.setColour(juce::Colour(0xFF383838));
        g.fillEllipse(cx-pivR, adjustY-pivR, pivR*2, pivR*2);
        g.setColour(juce::Colour(0xFF606060));
        g.drawEllipse(cx-pivR, adjustY-pivR, pivR*2, pivR*2, 0.8f);
        g.setColour(juce::Colour(0xFF505050));
        g.fillEllipse(cx-2, adjustY-2, 4, 4);

        // Glass reflection
        if (tex != nullptr && !tex->glassReflection.isNull())
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

        g.setColour(juce::Colour(0xFF404040));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.5f, 1.0f);
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
//==============================================================================
class LEDLadderMeter : public juce::Component
{
public:
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

        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colour(0xFF000000).withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        float levelDb = juce::Decibels::gainToDecibels(level, -60.0f);
        float peakDb  = juce::Decibels::gainToDecibels(peak, -60.0f);

        const float ledDb[] = {
            -54.0f,-48.0f,-42.0f,-36.0f,-30.0f,
            -24.0f,-18.0f,-15.0f,-12.0f,-9.0f,
            -6.0f,-3.0f,0.0f,3.0f };
        const int numLeds = 14;

        float ledH = (h - 6.0f) / numLeds;
        float ledW = w - 6.0f;
        float ledGap = 1.5f;

        for (int i = 0; i < numLeds; i++)
        {
            int   idx    = numLeds - 1 - i;
            float ly     = 3.0f + i * ledH;
            bool  active = levelDb >= ledDb[idx];
            bool  isPeak = std::abs(peakDb - ledDb[idx]) < 3.5f
                        && peakHold > 0 && !active;

            juce::Colour ledOn, ledOff;
            if (ledDb[idx] >= 3.0f)
            {
                ledOn = juce::Colour(0xFFC83020);
                ledOff = juce::Colour(0xFF180808);
            }
            else if (ledDb[idx] >= -3.0f)
            {
                ledOn = juce::Colour(0xFFE84030);
                ledOff = juce::Colour(0xFF180808);
            }
            else if (ledDb[idx] >= -9.0f)
            {
                ledOn = juce::Colour(0xFFB8A020);
                ledOff = juce::Colour(0xFF141408);
            }
            else
            {
                ledOn = juce::Colour(0xFF3A8A3A);
                ledOff = juce::Colour(0xFF081408);
            }

            auto ledRect = juce::Rectangle<float>(
                3.0f, ly, ledW, ledH - ledGap);

            if (active)
            {
                g.setColour(ledOn.withAlpha(0.12f));
                g.fillRoundedRectangle(ledRect.expanded(1.5f), 2.0f);
                juce::ColourGradient grad(
                    ledOn.brighter(0.2f), ledRect.getX(), ledRect.getY(),
                    ledOn.darker(0.3f), ledRect.getX(), ledRect.getBottom(), false);
                g.setGradientFill(grad);
                g.fillRoundedRectangle(ledRect, 1.2f);
                juce::ColourGradient dome(
                    juce::Colours::white.withAlpha(0.18f),
                    ledRect.getX()+ledW*0.2f, ledRect.getY(),
                    juce::Colours::transparentWhite,
                    ledRect.getX()+ledW*0.5f, ledRect.getBottom(), false);
                g.setGradientFill(dome);
                g.fillRoundedRectangle(ledRect, 1.2f);
            }
            else
            {
                g.setColour(ledOff);
                g.fillRoundedRectangle(ledRect, 1.2f);
            }

            if (isPeak)
            {
                g.setColour(ledOn.withAlpha(0.65f));
                g.fillRoundedRectangle(ledRect, 1.2f);
            }
        }

        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
        g.setColour(juce::Colour(0xFF252525));
        g.drawHorizontalLine(
            static_cast<int>(bounds.getY()) + 1,
            bounds.getX() + 2, bounds.getRight() - 2);
    }

private:
    float level    = 0.0f;
    float peak     = 0.0f;
    int   peakHold = 0;
};

//==============================================================================
// A/B COMPARISON SLOTS
// Store state A and B, switch instantly
//==============================================================================
class ABComparison : public juce::Component
{
public:
    std::function<void(bool isA)> onSwitch;
    std::function<void(bool isA)> onStore;

    ABComparison()
    {
        storeABtn.setButtonText("STORE A");
        storeBBtn.setButtonText("STORE B");
        switchABtn.setButtonText("A");
        switchBBtn.setButtonText("B");

        auto setupBtn = [this](juce::TextButton& btn)
        {
            btn.setColour(juce::TextButton::buttonColourId,
                juce::Colour(0xFF111111));
            btn.setColour(juce::TextButton::textColourOffId,
                juce::Colour(0xFFE8A838));
            addAndMakeVisible(btn);
        };

        setupBtn(storeABtn);
        setupBtn(storeBBtn);
        setupBtn(switchABtn);
        setupBtn(switchBBtn);

        storeABtn.onClick  = [this] {
            hasA = true;
            if (onStore) onStore(true);
            repaint();
        };
        storeBBtn.onClick  = [this] {
            hasB = true;
            if (onStore) onStore(false);
            repaint();
        };
        switchABtn.onClick = [this] {
            if (hasA && onSwitch) onSwitch(true);
        };
        switchBBtn.onClick = [this] {
            if (hasB && onSwitch) onSwitch(false);
        };
    }

    void resized() override
    {
        auto b = getLocalBounds();
        int w4 = b.getWidth() / 4;
        storeABtn .setBounds(0, 0, w4-1, b.getHeight());
        switchABtn.setBounds(w4, 0, w4-1, b.getHeight());
        switchBBtn.setBounds(w4*2, 0, w4-1, b.getHeight());
        storeBBtn .setBounds(w4*3, 0, w4, b.getHeight());
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xFF0A0A0A));
        g.fillRoundedRectangle(
            getLocalBounds().toFloat(), 3.0f);

        // Indicate which slots are filled
        auto b = getLocalBounds();
        if (hasA)
        {
            g.setColour(juce::Colour(0xFFE8A838)
                .withAlpha(0.15f));
            g.fillRect(b.getX(), b.getY(),
                b.getWidth()/2, b.getHeight());
        }
        if (hasB)
        {
            g.setColour(juce::Colour(0xFFE8A838)
                .withAlpha(0.15f));
            g.fillRect(b.getX() + b.getWidth()/2,
                b.getY(), b.getWidth()/2, b.getHeight());
        }
    }

private:
    juce::TextButton storeABtn;
    juce::TextButton storeBBtn;
    juce::TextButton switchABtn;
    juce::TextButton switchBBtn;
    bool hasA = false;
    bool hasB = false;
};

//==============================================================================
// LUFS METER
// Short-term LUFS display (3-second integration)
// Numeric readout updated at 10Hz
//==============================================================================
class LUFSMeter : public juce::Component
{
public:
	void setCurrentLUFS(float lufs)
    {
        currentLUFS = lufs;
    }

    void updateSamples(const float* L, const float* R,
                       int numSamples, double sampleRate)
    {
        // K-weighted momentary LUFS approximation
        // Simple RMS-based estimate with K-weighting factor
        // Full ITU-R BS.1770 requires pre-filter + 400ms gating
        // This is a simplified display-only approximation

        float sumSq = 0.0f;
        for (int i = 0; i < numSamples; i++)
        {
            float lSamp = L ? L[i] : 0.0f;
            float rSamp = R ? R[i] : 0.0f;
            sumSq += lSamp*lSamp + rSamp*rSamp;
        }

        float rms = std::sqrt(
            sumSq / (2.0f * static_cast<float>(numSamples)));

        // Smooth with 3-second time constant
        float coeff = std::exp(-1.0f /
            (static_cast<float>(sampleRate) * 3.0f
             / static_cast<float>(numSamples)));

        smoothedRMS = coeff * smoothedRMS
            + (1.0f - coeff) * rms;

        // K-weighting approx (+4dB high shelf character)
        float lufs = -0.691f
            + 10.0f * std::log10(smoothedRMS * smoothedRMS
            * 2.0f + 1e-10f);

        currentLUFS = lufs;
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        g.setColour(juce::Colour(0xFF0A0A0A));
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);

        // LUFS value
        juce::String lufsStr;
        if (currentLUFS < -70.0f)
            lufsStr = "---";
        else
            lufsStr = juce::String(currentLUFS, 1);

        g.setColour(juce::Colour(0xFFE8A838));
        g.setFont(juce::Font(juce::FontOptions(9.0f)
            .withStyle("Bold")));
        g.drawText(lufsStr,
            b.toNearestInt(), juce::Justification::centred);

        // Label
        g.setColour(juce::Colour(0xFF555555));
        g.setFont(juce::Font(juce::FontOptions(6.0f)));
        g.drawText("LUFS",
            static_cast<int>(b.getX()),
            static_cast<int>(b.getBottom()) - 10,
            static_cast<int>(b.getWidth()), 10,
            juce::Justification::centred);
    }

    float getCurrentLUFS() const { return currentLUFS; }

private:
    float smoothedRMS  = 0.0f;
    float currentLUFS  = -70.0f;
};

//==============================================================================
// ANALOG LOOK AND FEEL
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
        setColour(juce::PopupMenu::highlightedBackgroundColourId,
            juce::Colour(0xFF1A1A10));
        setColour(juce::PopupMenu::highlightedTextColourId,
            juce::Colour(0xFFE8A838));
        setColour(juce::Label::textColourId,
            juce::Colour(0xFFD8D0C0));
    }

    void drawRotarySlider(
        juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(
            x, y, width, height).toFloat().reduced(4);
        float radius  = juce::jmin(
            bounds.getWidth(), bounds.getHeight()) * 0.5f;
        float centreX = bounds.getCentreX();
        float centreY = bounds.getCentreY();
        float angle   = rotaryStartAngle + sliderPos
            * (rotaryEndAngle - rotaryStartAngle);
        bool  enabled = slider.isEnabled();
        float alpha   = enabled ? 1.0f : 0.25f;

        int styleInt = static_cast<int>(
            slider.getProperties().getWithDefault(
                "knobStyle", 0));

        auto* tex = TextureCache::getInstance();
        bool drawnFromStrip = false;

        if (tex != nullptr)
        {
            juce::Image* strip = nullptr;
            switch (styleInt)
            {
                case 1: strip = &tex->knobFilmstripVintage; break;
                case 2: strip = &tex->knobFilmstripSSL;     break;
                case 3: strip = &tex->knobFilmstripAPI;      break;
                default: strip = &tex->knobFilmstrip;        break;
            }

            if (strip != nullptr && !strip->isNull()
             && strip->getWidth() > 0)
            {
                int numFrames = strip->getHeight()
                    / strip->getWidth();
                int frame = juce::jlimit(0, numFrames-1,
                    static_cast<int>(
                        sliderPos * (numFrames-1)));
                int frameH = strip->getWidth();
                int frameY = frame * frameH;
                int kSize  = static_cast<int>(radius*2.0f);
                int kX = static_cast<int>(centreX-radius);
                int kY = static_cast<int>(centreY-radius);

                g.setOpacity(alpha);
                g.drawImage(*strip, kX, kY, kSize, kSize,
                    0, frameY, frameH, frameH, false);
                g.setOpacity(1.0f);
                drawnFromStrip = true;
            }
        }

        if (!drawnFromStrip)
        {
            juce::ColourGradient bodyGrad(
                juce::Colour(0xFF3A3A3A).withAlpha(alpha),
                centreX, centreY-radius,
                juce::Colour(0xFF181818).withAlpha(alpha),
                centreX, centreY+radius, false);
            g.setGradientFill(bodyGrad);
            g.fillEllipse(centreX-radius, centreY-radius,
                radius*2, radius*2);
            if (enabled)
            {
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                g.setColour(juce::Colour(0xFFE8A838));
                g.drawLine(centreX, centreY,
                    centreX+radius*0.65f*cosA,
                    centreY+radius*0.65f*sinA, 2.0f);
            }
        }

        // Track arc
        juce::Path trackArc;
        trackArc.addCentredArc(centreX, centreY,
            radius*0.9f, radius*0.9f, 0.0f,
            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF1A1A1A).withAlpha(alpha));
        g.strokePath(trackArc, juce::PathStrokeType(2.5f));

        if (sliderPos > 0.005f && enabled)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY,
                radius*0.9f, radius*0.9f, 0.0f,
                rotaryStartAngle, angle, true);
            g.setColour(juce::Colour(0xFFE8A838).withAlpha(0.80f));
            g.strokePath(activeArc, juce::PathStrokeType(3.0f,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));
        }
    }

    void drawComboBox(
        juce::Graphics& g,
        int width, int height,
        bool isDown, int, int, int, int,
        juce::ComboBox&) override
    {
        auto b = juce::Rectangle<float>(
            0, 0, static_cast<float>(width),
            static_cast<float>(height));
        g.setColour(juce::Colour(isDown ? 0xFF181818 : 0xFF111111));
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(b.reduced(0.5f), 3.0f, 1.0f);
        g.setColour(juce::Colour(0xFF1E1E1E));
        g.drawHorizontalLine(1, 2.0f, (float)width-2.0f);
        float arrowX = (float)width - 16.0f;
        float arrowY = (float)height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle(arrowX, arrowY-3.0f,
            arrowX+7.0f, arrowY, arrowX, arrowY+3.0f);
        g.setColour(juce::Colour(0xFFE8A838));
        g.fillPath(arrow);
    }

    void drawToggleButton(
        juce::Graphics& g,
        juce::ToggleButton& button,
        bool highlighted, bool) override
    {
        auto b = button.getLocalBounds().toFloat().reduced(1.0f);
        bool isBypass = static_cast<bool>(
            button.getProperties().getWithDefault(
                "isBypass", false));
        bool on = isBypass
            ? !button.getToggleState()
            :  button.getToggleState();

        g.setColour(on
            ? juce::Colour(0xFF1A1200)
            : juce::Colour(0xFF0F0F0F));
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF222222));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);

        float dotX = b.getX() + 8.0f;
        float dotY = b.getCentreY();
        float dotR = 2.5f;
        if (on)
        {
            g.setColour(juce::Colour(0x28E8A838));
            g.fillEllipse(dotX-5.0f, dotY-5.0f, 10.0f, 10.0f);
            g.setColour(juce::Colour(0xFFE8A838));
        }
        else
            g.setColour(juce::Colour(0xFF252525));
        g.fillEllipse(dotX-dotR, dotY-dotR, dotR*2.0f, dotR*2.0f);

        g.setColour(on
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF3A3A3A));
        g.setFont(juce::Font(juce::FontOptions(8.0f).withStyle("Bold")));
        g.drawText(button.getButtonText(),
            b.withLeft(b.getX()+16.0f),
            juce::Justification::centredLeft);

        if (highlighted)
        {
            g.setColour(juce::Colours::white.withAlpha(0.02f));
            g.fillRoundedRectangle(b, 3.0f);
        }
    }

    void drawLabel(
        juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(
            juce::Label::textColourId));
        g.setFont(label.getFont());
        g.drawText(label.getText(),
            label.getLocalBounds(),
            label.getJustificationType());
    }
};