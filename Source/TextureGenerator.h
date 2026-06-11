#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <random>

//==============================================================================
// TEXTURE CACHE
// Uses JUCE's DeletedAtShutdown + singleton pattern
// This ensures images are destroyed BEFORE JUCE tears down
// The raw static singleton pattern causes crash-on-close
// because juce::Image ref counting walks freed memory
//==============================================================================
class TextureCache : public juce::DeletedAtShutdown
{
public:
    JUCE_DECLARE_SINGLETON(TextureCache, false)

    // All cached textures as public members
    juce::Image brushedMetalDark;
    juce::Image brushedMetalGrey;
    juce::Image brushedMetalSilver;
    juce::Image wrinkleFinish;
    juce::Image creamPanel;
    juce::Image woodGrain;
    juce::Image knobFilmstrip;
    juce::Image knobFilmstripVintage;
    juce::Image knobFilmstripSSL;
    juce::Image knobFilmstripAPI;
    juce::Image vuFaceCream;
    juce::Image vuFaceBlack;
    juce::Image vuFaceWhite;
    juce::Image scratchOverlay;
    juce::Image dustOverlay;
    juce::Image glassReflection;
    juce::Image screwHead;
    juce::Image headerBackground;

    TextureCache()
    {
        // Generate at 256x256 not 512x512
        // Quarter the pixel count same visual result
        int W = 256, H = 256;

        brushedMetalDark   = makeBrushedMetal(
            W, H, 0xFF141414, 0.04f);
        brushedMetalGrey   = makeBrushedMetal(
            W, H, 0xFF3A3C40, 0.05f);
        brushedMetalSilver = makeBrushedMetal(
            W, H, 0xFFB0B4B8, 0.06f);
        wrinkleFinish      = makeWrinkleFinish(
            W, H, 0xFF1C1410);
        creamPanel         = makeCreamPanel(W, H);
        woodGrain          = makeWoodGrain(128, 640);

        int kSize = 80;
        knobFilmstrip        = makeKnobFilmstrip(
            kSize, 100, KnobStyle::DarkMetal);
        knobFilmstripVintage = makeKnobFilmstrip(
            kSize, 100, KnobStyle::VintageDome);
        knobFilmstripSSL     = makeKnobFilmstrip(
            kSize, 100, KnobStyle::SSLCompact);
        knobFilmstripAPI     = makeKnobFilmstrip(
            kSize, 100, KnobStyle::APISkirt);

        vuFaceCream = makeVUFace(240, 130,
            juce::Colour(0xFFE8DCCC),
            juce::Colour(0xFF2A1A0A));
        vuFaceBlack = makeVUFace(240, 130,
            juce::Colour(0xFF050505),
            juce::Colour(0xFFD0D0D0));
        vuFaceWhite = makeVUFace(240, 130,
            juce::Colour(0xFFF0F0F0),
            juce::Colour(0xFF1A1A1A));

        scratchOverlay  = makeScratchOverlay(W, H);
        dustOverlay     = makeDustOverlay(W, H);
        glassReflection = makeGlassReflection(W, H);
        screwHead       = makeScrewHeadImage(32);
        headerBackground = makeHeaderBackground(
            1280, 48);
    }

    ~TextureCache()
    {
        clearSingletonInstance();
    }

private:

    static juce::Image makeBrushedMetal(
        int w, int h,
        uint32_t baseColorARGB,
        float intensity)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);

        juce::Colour base(baseColorARGB);
        g.fillAll(base);

        std::mt19937 rng(42);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);

        for (int i = 0; i < 200; i++)
        {
            float y  = dist(rng) * h;
            float x1 = dist(rng) * w * 0.2f;
            float x2 = x1 + dist(rng) * w * 0.8f;
            float a  = 0.01f + dist(rng) * intensity;
            g.setColour(juce::Colours::white
                .withAlpha(a));
            g.drawLine(x1, y, x2, y, 0.4f);
        }

        for (int i = 0; i < 6; i++)
        {
            float y  = dist(rng) * h;
            float x1 = dist(rng) * w * 0.3f;
            float x2 = x1 + dist(rng) * w * 0.4f;
            float a  = 0.03f + dist(rng) * 0.04f;
            g.setColour(juce::Colours::white
                .withAlpha(a));
            g.drawLine(x1, y, x2, y, 0.8f);
        }

        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.04f),
            0, 0,
            juce::Colours::black.withAlpha(0.06f),
            0, (float)h, false));
        g.fillRect(0, 0, w, h);

        return img;
    }

    static juce::Image makeWrinkleFinish(
        int w, int h, uint32_t baseColorARGB)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);

        g.fillAll(juce::Colour(baseColorARGB));

        std::mt19937 rng(12345);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);

        for (int i = 0; i < 1000; i++)
        {
            float x = dist(rng) * w;
            float y = dist(rng) * h;
            float s = 1.5f + dist(rng) * 4.0f;
            float a = 0.02f + dist(rng) * 0.04f;
            bool lighter = dist(rng) > 0.5f;
            g.setColour(lighter
                ? juce::Colours::white.withAlpha(a)
                : juce::Colours::black
                    .withAlpha(a * 1.5f));
            g.fillEllipse(x, y, s, s * 0.6f);
        }

        return img;
    }

    static juce::Image makeCreamPanel(int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colour(0xFFE8E0C8));

        std::mt19937 rng(789);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);
        for (int i = 0; i < 1500; i++)
        {
            float x = dist(rng) * w;
            float y = dist(rng) * h;
            float s = 0.5f + dist(rng) * 1.5f;
            float a = 0.005f + dist(rng) * 0.012f;
            g.setColour(juce::Colour(0xFF8B7355)
                .withAlpha(a));
            g.fillEllipse(x, y, s, s);
        }
        return img;
    }

    static juce::Image makeWoodGrain(int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colour(0xFF3C2A18));

        std::mt19937 rng(55);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);
        std::normal_distribution<float>
            nd(0.0f, 2.0f);

        for (int i = 0; i < 14; i++)
        {
            float baseX = (float)i / 14.0f * w
                        + dist(rng) * 3.0f;
            float lineW  = 0.8f + dist(rng) * 1.5f;
            bool lighter = dist(rng) > 0.4f;

            juce::Path path;
            path.startNewSubPath(baseX, 0);
            float x = baseX;
            for (int y = 0; y <= h; y += 8)
            {
                x += nd(rng) * 0.3f;
                path.lineTo(x, (float)y);
            }

            float a = 0.12f + dist(rng) * 0.18f;
            g.setColour(lighter
                ? juce::Colour(0xFF5A3E24)
                    .withAlpha(a)
                : juce::Colour(0xFF2A1A08)
                    .withAlpha(a));
            g.strokePath(path,
                juce::PathStrokeType(lineW));
        }

        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.05f),
            0, 0,
            juce::Colours::transparentWhite,
            (float)w, 0, false));
        g.fillRect(0, 0, w, h);

        return img;
    }

    enum class KnobStyle
    {
        DarkMetal,
        VintageDome,
        SSLCompact,
        APISkirt
    };

    static juce::Image makeKnobFilmstrip(
        int kSize, int numFrames,
        KnobStyle style)
    {
        juce::Image strip(juce::Image::ARGB,
            kSize, kSize * numFrames, true);
        juce::Graphics g(strip);

        float startAngle = -2.356f;
        float totalSweep = 4.712f;

        for (int frame = 0; frame < numFrames; frame++)
        {
            float pos = static_cast<float>(frame)
                / (numFrames - 1);
            float angle = startAngle + pos * totalSweep;
            int   frameY = frame * kSize;

            float cx = kSize * 0.5f;
            float cy = frameY + kSize * 0.5f;
            float outerR = kSize * 0.48f;
            float capR   = kSize * 0.34f;

            switch (style)
            {
                case KnobStyle::DarkMetal:
                    drawDarkMetal(g, cx, cy,
                        outerR, capR, angle,
                        frame, numFrames);
                    break;
                case KnobStyle::VintageDome:
                    drawVintageDome(g, cx, cy,
                        outerR, capR, angle);
                    break;
                case KnobStyle::SSLCompact:
                    drawSSLCompact(g, cx, cy,
                        outerR, capR * 0.85f, angle);
                    break;
                case KnobStyle::APISkirt:
                    drawAPISkirt(g, cx, cy,
                        outerR, capR, angle);
                    break;
            }
        }
        return strip;
    }

    static void drawDarkMetal(
        juce::Graphics& g,
        float cx, float cy,
        float outerR, float capR,
        float angle, int frame, int numFrames)
    {
        float startAngle = -2.356f;
        float totalSweep = 4.712f;

        // Outer shadow
        g.setColour(juce::Colour(0x25000000));
        g.fillEllipse(cx-outerR+1, cy-outerR+1,
            outerR*2, outerR*2);

        // Track ring
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawEllipse(cx-outerR, cy-outerR,
            outerR*2, outerR*2, 3.0f);

        // Active arc
        if (frame > 0)
        {
            float pos = static_cast<float>(frame)
                / (numFrames - 1);
            float endAngle = startAngle
                + pos * totalSweep;
            juce::Path arc;
            arc.addCentredArc(cx, cy,
                outerR-1.5f, outerR-1.5f,
                0.0f, startAngle, endAngle, true);
            g.setColour(juce::Colour(0xFFE8A838)
                .withAlpha(0.75f));
            g.strokePath(arc,
                juce::PathStrokeType(3.0f,
                    juce::PathStrokeType::curved,
                    juce::PathStrokeType::rounded));
        }

        // Cap gradient
        juce::ColourGradient capGrad(
            juce::Colour(0xFF3A3A3A),
            cx, cy-capR,
            juce::Colour(0xFF181818),
            cx, cy+capR, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(cx-capR, cy-capR,
            capR*2, capR*2);

        // Cap ring
        g.setColour(juce::Colour(0xFF444444));
        g.drawEllipse(cx-capR, cy-capR,
            capR*2, capR*2, 1.2f);

        // Knurling
        int numDots = 18;
        float dotR = capR + 2.0f;
        for (int i = 0; i < numDots; i++)
        {
            float da = static_cast<float>(i)
                / numDots
                * juce::MathConstants<float>::twoPi;
            float dx = cx + dotR * std::cos(da);
            float dy = cy + dotR * std::sin(da);
            g.setColour(juce::Colour(0xFF252525));
            g.fillEllipse(dx-0.8f, dy-0.8f,
                1.6f, 1.6f);
        }

        // Indicator
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        g.setColour(juce::Colour(0xFF000000)
            .withAlpha(0.4f));
        g.drawLine(cx + 0.4f, cy + 0.4f,
            cx + capR*0.78f*cosA + 0.4f,
            cy + capR*0.78f*sinA + 0.4f, 2.0f);
        g.setColour(juce::Colour(0xFFE8A838));
        g.drawLine(cx, cy,
            cx + capR*0.78f*cosA,
            cy + capR*0.78f*sinA, 2.0f);

        // Center dot
        g.setColour(juce::Colour(0xFF404040));
        g.fillEllipse(cx-2.5f, cy-2.5f, 5, 5);

        // Highlight
        g.setColour(juce::Colour(0x08FFFFFF));
        g.fillEllipse(cx-capR*0.35f, cy-capR*0.55f,
            capR*0.7f, capR*0.35f);
    }

    static void drawVintageDome(
        juce::Graphics& g,
        float cx, float cy,
        float outerR, float capR,
        float angle)
    {
        // Chrome ring
        juce::ColourGradient ringGrad(
            juce::Colour(0xFF888888),
            cx, cy-outerR,
            juce::Colour(0xFF303030),
            cx, cy+outerR, false);
        g.setGradientFill(ringGrad);
        g.fillEllipse(cx-outerR, cy-outerR,
            outerR*2, outerR*2);

        // Bakelite dome
        juce::ColourGradient domeGrad(
            juce::Colour(0xFF2A1808),
            cx-capR*0.3f, cy-capR*0.6f,
            juce::Colour(0xFF0A0800),
            cx+capR*0.5f, cy+capR*0.8f, false);
        g.setGradientFill(domeGrad);
        g.fillEllipse(cx-capR, cy-capR,
            capR*2, capR*2);

        // Dome highlight
        juce::ColourGradient dome(
            juce::Colours::white.withAlpha(0.18f),
            cx-capR*0.35f, cy-capR*0.55f,
            juce::Colours::transparentWhite,
            cx+capR*0.3f, cy+capR*0.4f, false);
        g.setGradientFill(dome);
        g.fillEllipse(cx-capR, cy-capR,
            capR*2, capR*2);

        // Indicator
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        g.setColour(juce::Colour(0xFFFFFFEE)
            .withAlpha(0.9f));
        g.drawLine(cx, cy,
            cx + capR*0.85f*cosA,
            cy + capR*0.85f*sinA, 2.2f);
    }

    static void drawSSLCompact(
        juce::Graphics& g,
        float cx, float cy,
        float outerR, float capR,
        float angle)
    {
        // Shadow
        g.setColour(juce::Colour(0x20000000));
        g.fillEllipse(cx-outerR+1, cy-outerR+1,
            outerR*2, outerR*2);

        // Track
        g.setColour(juce::Colour(0xFF1A1A1A));
        g.drawEllipse(cx-outerR, cy-outerR,
            outerR*2, outerR*2, 2.5f);

        // Cap
        juce::ColourGradient capGrad(
            juce::Colour(0xFF282828),
            cx-capR, cy-capR,
            juce::Colour(0xFF161616),
            cx+capR, cy+capR, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(cx-capR, cy-capR,
            capR*2, capR*2);
        g.setColour(juce::Colour(0xFF303030));
        g.drawEllipse(cx-capR, cy-capR,
            capR*2, capR*2, 1.0f);

        // Indicator
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        g.setColour(juce::Colours::white
            .withAlpha(0.85f));
        g.drawLine(cx, cy,
            cx + capR*0.85f*cosA,
            cy + capR*0.85f*sinA, 1.5f);

        // Center
        g.setColour(juce::Colour(0xFF3A3A3A));
        g.fillEllipse(cx-2, cy-2, 4, 4);
    }

    static void drawAPISkirt(
        juce::Graphics& g,
        float cx, float cy,
        float outerR, float capR,
        float angle)
    {
        // Shadow
        g.setColour(juce::Colour(0x28000000));
        g.fillEllipse(cx-outerR+1, cy-outerR+1,
            outerR*2, outerR*2);

        // Machined aluminum skirt
        juce::ColourGradient skirtGrad(
            juce::Colour(0xFF4A4A4A),
            cx-outerR, cy,
            juce::Colour(0xFF282828),
            cx+outerR, cy, false);
        g.setGradientFill(skirtGrad);
        g.fillEllipse(cx-outerR, cy-outerR,
            outerR*2, outerR*2);

        // Skirt lines
        for (int i = 0; i < 20; i++)
        {
            float a = static_cast<float>(i)
                / 20.0f
                * juce::MathConstants<float>::twoPi;
            float ix = cx + (outerR-3) * std::cos(a);
            float iy = cy + (outerR-3) * std::sin(a);
            float ox = cx + outerR * std::cos(a);
            float oy = cy + outerR * std::sin(a);
            g.setColour(juce::Colour(0xFF1A1A1A)
                .withAlpha(0.4f));
            g.drawLine(ix, iy, ox, oy, 0.8f);
        }

        // Inner cap
        juce::ColourGradient capGrad(
            juce::Colour(0xFF222222),
            cx-capR, cy-capR,
            juce::Colour(0xFF111111),
            cx+capR, cy+capR, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(cx-capR, cy-capR,
            capR*2, capR*2);
        g.setColour(juce::Colour(0xFF333333));
        g.drawEllipse(cx-capR, cy-capR,
            capR*2, capR*2, 1.0f);

        // Bold amber indicator
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        g.setColour(juce::Colour(0xFF000000)
            .withAlpha(0.4f));
        g.drawLine(cx+0.5f, cy+0.5f,
            cx+capR*0.75f*cosA+0.5f,
            cy+capR*0.75f*sinA+0.5f, 3.0f);
        g.setColour(juce::Colour(0xFFE8A838));
        g.drawLine(cx, cy,
            cx+capR*0.75f*cosA,
            cy+capR*0.75f*sinA, 3.0f);

        // Center bolt
        g.setColour(juce::Colour(0xFF888888));
        g.fillEllipse(cx-4, cy-4, 8, 8);
        g.setColour(juce::Colour(0xFF444444));
        g.drawLine(cx-2.5f, cy, cx+2.5f, cy, 1.0f);
    }

    static juce::Image makeVUFace(
        int w, int h,
        juce::Colour faceColor,
        juce::Colour textColor)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);

        g.fillAll(faceColor);

        // Paper texture
        std::mt19937 rng(999);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);
        for (int i = 0; i < 1000; i++)
        {
            float x = dist(rng) * w;
            float y = dist(rng) * h;
            float a = 0.004f + dist(rng) * 0.006f;
            g.setColour(dist(rng) > 0.5f
                ? juce::Colours::black.withAlpha(a)
                : juce::Colours::white.withAlpha(a));
            g.fillEllipse(x, y, 0.8f, 0.8f);
        }

        // Scale marks
        float cx = w * 0.5f;
        float pivotY = h * 0.92f;
        float arcR = w * 0.38f;
        float startAngle = -2.35f;
        float sweepAngle = 4.7f;

        g.setColour(textColor);
        g.setFont(juce::Font(juce::FontOptions(7.0f)));

        const float markPos[] = {
            0.0f, 0.12f, 0.22f, 0.34f,
            0.48f, 0.64f, 0.8f, 1.0f };
        const char* markLbl[] = {
            "0", "-2", "-4", "-6",
            "-10", "-14", "-20", "" };

        for (int i = 0; i < 8; i++)
        {
            float a = startAngle
                + markPos[i] * sweepAngle;
            float cosA = std::cos(a);
            float sinA = std::sin(a);
            float adjY = pivotY - h * 0.12f;

            float ox = cx + arcR * cosA;
            float oy = adjY + arcR * sinA;
            float ix = cx + arcR*0.84f * cosA;
            float iy = adjY + arcR*0.84f * sinA;

            g.setColour(textColor.withAlpha(0.8f));
            g.drawLine(ix, iy, ox, oy, 0.9f);

            if (markLbl[i][0] != '\0')
                g.drawText(markLbl[i],
                    static_cast<int>(ox - 10),
                    static_cast<int>(oy - 12),
                    20, 10,
                    juce::Justification::centred);
        }

        // Red zone
        juce::Path red;
        red.addCentredArc(cx, pivotY - h*0.12f,
            arcR*0.92f, arcR*0.92f,
            0.0f, startAngle,
            startAngle + 0.22f*sweepAngle, true);
        g.setColour(juce::Colour(0xFFC83020)
            .withAlpha(0.25f));
        g.strokePath(red, juce::PathStrokeType(3.5f));

        // Label
        g.setColour(textColor.withAlpha(0.4f));
        g.setFont(juce::Font(juce::FontOptions(5.5f)));
        g.drawText("GAIN REDUCTION  dB",
            0, h - 14, w, 12,
            juce::Justification::centred);

        // Glass sheen
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.10f),
            0, 0,
            juce::Colours::transparentWhite,
            0, (float)h*0.5f, false));
        g.fillRect(0, 0, w, h);

        return img;
    }

    static juce::Image makeScratchOverlay(int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colours::transparentBlack);

        std::mt19937 rng(333);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);

        for (int i = 0; i < 20; i++)
        {
            float y  = dist(rng) * h;
            float x1 = dist(rng) * w * 0.4f;
            float x2 = x1 + dist(rng) * w * 0.4f;
            float a  = 0.02f + dist(rng) * 0.04f;
            g.setColour(juce::Colours::white
                .withAlpha(a));
            g.drawLine(x1, y, x2, y, 0.4f);
        }
        return img;
    }

    static juce::Image makeDustOverlay(int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colours::transparentBlack);

        std::mt19937 rng(777);
        std::uniform_real_distribution<float>
            dist(0.0f, 1.0f);

        for (int i = 0; i < 80; i++)
        {
            float x = dist(rng) * w;
            float y = dist(rng) * h;
            float s = 0.3f + dist(rng) * 1.0f;
            float a = 0.005f + dist(rng) * 0.01f;
            g.setColour(juce::Colours::white
                .withAlpha(a));
            g.fillEllipse(x, y, s, s);
        }
        return img;
    }

    static juce::Image makeGlassReflection(int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colours::transparentBlack);

        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.06f),
            0, 0,
            juce::Colours::transparentWhite,
            0, (float)h*0.35f, false));
        g.fillRect(0, 0, w, h);
        return img;
    }

    static juce::Image makeScrewHeadImage(int size)
    {
        juce::Image img(
            juce::Image::ARGB, size, size, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colours::transparentBlack);

        float cx = size * 0.5f;
        float cy = size * 0.5f;
        float r  = size * 0.45f;

        juce::ColourGradient outerGrad(
            juce::Colour(0xFF5A5A5A),
            cx-r, cy-r,
            juce::Colour(0xFF303030),
            cx+r, cy+r, false);
        g.setGradientFill(outerGrad);
        g.fillEllipse(cx-r, cy-r, r*2, r*2);

        juce::ColourGradient innerGrad(
            juce::Colour(0xFF484848),
            cx-r*0.6f, cy-r*0.6f,
            juce::Colour(0xFF282828),
            cx+r*0.6f, cy+r*0.6f, false);
        g.setGradientFill(innerGrad);
        g.fillEllipse(cx-r*0.85f, cy-r*0.85f,
            r*1.7f, r*1.7f);

        float slotLen = r * 0.55f;
        float slotW   = r * 0.18f;
        g.setColour(juce::Colour(0xFF141414));

        juce::Path hSlot;
        hSlot.addRoundedRectangle(
            cx-slotLen, cy-slotW*0.5f,
            slotLen*2, slotW, slotW*0.3f);
        g.fillPath(hSlot);

        juce::Path vSlot;
        vSlot.addRoundedRectangle(
            cx-slotW*0.5f, cy-slotLen,
            slotW, slotLen*2, slotW*0.3f);
        g.fillPath(vSlot);

        g.setColour(juce::Colours::white
            .withAlpha(0.12f));
        g.fillEllipse(cx-r*0.4f, cy-r*0.5f,
            r*0.8f, r*0.35f);

        g.setColour(juce::Colour(0xFF000000)
            .withAlpha(0.3f));
        g.drawEllipse(cx-r, cy-r, r*2, r*2, 1.0f);

        return img;
    }

    static juce::Image makeHeaderBackground(
        int w, int h)
    {
        juce::Image img(
            juce::Image::ARGB, w, h, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colour(0xFF0F0F0F));

        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.03f),
            0, 0,
            juce::Colours::black.withAlpha(0.04f),
            0, (float)h, false));
        g.fillRect(0, 0, w, h);

        return img;
    }
};