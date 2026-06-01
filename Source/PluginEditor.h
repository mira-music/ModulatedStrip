#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// VU METER COMPONENT
// Draws a vertical level meter
// Updated by timer at 30Hz
//==============================================================================
class VUMeter : public juce::Component
{
public:
    void setLevel(float newLevel)
    {
        // Smooth the meter ballistics
        // Fast attack, slow release
        if (newLevel > level)
            level = newLevel;
        else
            level = level * 0.85f + newLevel * 0.15f;

        // Peak hold
        if (newLevel > peak)
        {
            peak     = newLevel;
            peakHold = 60; // frames to hold peak
        }
        else if (peakHold > 0)
        {
            peakHold--;
        }
        else
        {
            peak = peak * 0.95f;
        }

        repaint();
    }

    void setIsGainReduction(bool gr)
    {
        isGainReduction = gr;
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Convert level to dB
        float levelDb = juce::Decibels::gainToDecibels(
            level, -60.0f);

        // Map dB to pixel height
        // -60dB = bottom, 0dB = top
        float normalized = (levelDb + 60.0f) / 60.0f;
        normalized = juce::jlimit(0.0f, 1.0f, normalized);

        float barH = bounds.getHeight() * normalized;
        float barY = bounds.getBottom() - barH;

        // Color based on level
        juce::Colour barColor;
        if (isGainReduction)
        {
            // GR meter is cyan/blue
            barColor = juce::Colour(0xFF00A8C8);
        }
        else
        {
            if (normalized < 0.7f)
                barColor = juce::Colour(0xFF3A8A3A);
            else if (normalized < 0.9f)
                barColor = juce::Colour(0xFFB8A020);
            else
                barColor = juce::Colour(0xFFC83020);
        }

        g.setColour(barColor);
        g.fillRoundedRectangle(
            bounds.getX() + 2,
            barY,
            bounds.getWidth() - 4,
            barH,
            2.0f);

        // Peak indicator line
        float peakDb = juce::Decibels::gainToDecibels(
            peak, -60.0f);
        float peakNorm = (peakDb + 60.0f) / 60.0f;
        peakNorm = juce::jlimit(0.0f, 1.0f, peakNorm);
        float peakY = bounds.getBottom()
                    - bounds.getHeight() * peakNorm;

        g.setColour(juce::Colours::white
            .withAlpha(0.6f));
        g.drawHorizontalLine(
            static_cast<int>(peakY),
            bounds.getX() + 2,
            bounds.getRight() - 2);

        // Border
        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }

private:
    float level          = 0.0f;
    float peak           = 0.0f;
    int   peakHold       = 0;
    bool  isGainReduction = false;
};

//==============================================================================
// BYPASS BUTTON
// Glows amber when active
// Goes dim when bypassed
//==============================================================================
class BypassButton : public juce::ToggleButton
{
public:
    BypassButton(const juce::String& label)
        : juce::ToggleButton(label)
    {
    }

    void paintButton(juce::Graphics& g,
                     bool highlighted,
                     bool) override
    {
        auto bounds = getLocalBounds()
            .toFloat().reduced(2);

        bool active = !getToggleState();

        // Background
        g.setColour(active
            ? juce::Colour(0xFF2A1A00)
            : juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Border - amber when active
        g.setColour(active
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF333333));
        g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

        // Text
        g.setColour(active
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF555555));
        g.setFont(juce::Font(
            juce::FontOptions(9.0f).withStyle("Bold")));
        g.drawText(getButtonText(),
            getLocalBounds(),
            juce::Justification::centred);

        // Highlight on hover
        if (highlighted)
        {
            g.setColour(juce::Colours::white
                .withAlpha(0.05f));
            g.fillRoundedRectangle(bounds, 4.0f);
        }
    }
};

//==============================================================================
// MAIN EDITOR
//==============================================================================
class ModulatedStripEditor
    : public juce::AudioProcessorEditor,
      public juce::Timer
{
public:
    ModulatedStripEditor(ModulatedStripProcessor&);
    ~ModulatedStripEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    ModulatedStripProcessor& processor;

    // INPUT
    juce::Slider inputGainSlider;
    juce::Label  inputGainLabel;

    // SATURATION
    juce::ComboBox  satModelSelector;
    juce::Slider    driveSlider;
    juce::Label     driveLabel;
    juce::Slider    satMixSlider;
    juce::Label     satMixLabel;
    BypassButton    satBypassBtn  { "ON" };

    // COMPRESSOR
    juce::ComboBox  compModelSelector;
    juce::Slider    thresholdSlider;
    juce::Label     thresholdLabel;
    juce::Slider    ratioSlider;
    juce::Label     ratioLabel;
    juce::Slider    attackSlider;
    juce::Label     attackLabel;
    juce::Slider    releaseSlider;
    juce::Label     releaseLabel;
    juce::Slider    makeupSlider;
    juce::Label     makeupLabel;
    juce::Slider    compMixSlider;
    juce::Label     compMixLabel;
    juce::Slider    kneeSlider;
    juce::Label     kneeLabel;
    BypassButton    compBypassBtn { "ON" };

    // EQ
    juce::ComboBox  eqModelSelector;
    juce::Slider    eqLowGainSlider;
    juce::Label     eqLowGainLabel;
    juce::Slider    eqLowFreqSlider;
    juce::Label     eqLowFreqLabel;
    juce::Slider    eqMidGainSlider;
    juce::Label     eqMidGainLabel;
    juce::Slider    eqMidFreqSlider;
    juce::Label     eqMidFreqLabel;
    juce::Slider    eqMidQSlider;
    juce::Label     eqMidQLabel;
    juce::Slider    eqHighGainSlider;
    juce::Label     eqHighGainLabel;
    juce::Slider    eqHighFreqSlider;
    juce::Label     eqHighFreqLabel;
    juce::Slider    eqHPFSlider;
    juce::Label     eqHPFLabel;
    BypassButton    eqBypassBtn   { "ON" };
    BypassButton    eqPreCompBtn  { "EQ PRE" };

    // OUTPUT
    juce::Slider    outputGainSlider;
    juce::Label     outputGainLabel;

    // METERS
    VUMeter inputMeter;
    VUMeter outputMeter;
    VUMeter grMeter;

    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;
    juce::Label grMeterLabel;

    //──────────────────────────────────────────────
    // PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────

    // Input
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> inputGainAttachment;

    // Saturation
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> satMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> satModelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ButtonAttachment> satBypassAttachment;

    // Compressor
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> makeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> compMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> compModelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ButtonAttachment> compBypassAttachment;

    // EQ
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqLowGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqLowFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqMidGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqMidFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqMidQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqHighGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqHighFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> eqHPFAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> eqModelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ButtonAttachment> eqBypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ButtonAttachment> eqPreCompAttachment;

    // Output
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> outputGainAttachment;

    void setupKnob(juce::Slider& slider,
                   juce::Label& label,
                   const juce::String& text);

    void setupCombo(juce::ComboBox& box,
                    const juce::StringArray& items);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};