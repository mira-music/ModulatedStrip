#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// VU METER
//==============================================================================
class VUMeter : public juce::Component
{
public:
    void setLevel(float newLevel)
    {
        if (newLevel > level)
            level = newLevel;
        else
            level = level * 0.85f + newLevel * 0.15f;

        if (newLevel > peak)
        {
            peak     = newLevel;
            peakHold = 60;
        }
        else if (peakHold > 0)
            peakHold--;
        else
            peak = peak * 0.95f;

        repaint();
    }

    void setIsGainReduction(bool gr)
    { isGainReduction = gr; }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        g.setColour(juce::Colour(0xFF080808));
        g.fillRoundedRectangle(b, 3.0f);

        float levelDb = juce::Decibels::gainToDecibels(
            level, -60.0f);
        float norm = juce::jlimit(0.0f, 1.0f,
            (levelDb + 60.0f) / 60.0f);

        float barH = b.getHeight() * norm;
        float barY = b.getBottom() - barH;

        juce::Colour col;
        if (isGainReduction)
            col = juce::Colour(0xFF00A8C8);
        else if (norm < 0.7f)
            col = juce::Colour(0xFF3A8A3A);
        else if (norm < 0.9f)
            col = juce::Colour(0xFFB8A020);
        else
            col = juce::Colour(0xFFC83020);

        g.setColour(col);
        g.fillRoundedRectangle(
            b.getX() + 2, barY,
            b.getWidth() - 4, barH, 2.0f);

        float peakDb = juce::Decibels::gainToDecibels(
            peak, -60.0f);
        float peakNorm = juce::jlimit(0.0f, 1.0f,
            (peakDb + 60.0f) / 60.0f);
        float peakY = b.getBottom()
                    - b.getHeight() * peakNorm;

        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawHorizontalLine(static_cast<int>(peakY),
            b.getX() + 2, b.getRight() - 2);

        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawRoundedRectangle(b, 3.0f, 1.0f);
    }

private:
    float level = 0.0f;
    float peak  = 0.0f;
    int   peakHold = 0;
    bool  isGainReduction = false;
};

//==============================================================================
// BYPASS BUTTON
//==============================================================================
class BypassButton : public juce::ToggleButton
{
public:
    BypassButton(const juce::String& label)
        : juce::ToggleButton(label) {}

    void paintButton(juce::Graphics& g,
                     bool highlighted, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced(2);
        bool active = !getToggleState();

        g.setColour(active
            ? juce::Colour(0xFF2A1A00)
            : juce::Colour(0xFF1A1A1A));
        g.fillRoundedRectangle(b, 4.0f);

        g.setColour(active
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF333333));
        g.drawRoundedRectangle(b, 4.0f, 1.5f);

        g.setColour(active
            ? juce::Colour(0xFFE8A838)
            : juce::Colour(0xFF555555));
        g.setFont(juce::Font(
            juce::FontOptions(9.0f).withStyle("Bold")));
        g.drawText(getButtonText(),
            getLocalBounds(),
            juce::Justification::centred);

        if (highlighted)
        {
            g.setColour(
                juce::Colours::white.withAlpha(0.05f));
            g.fillRoundedRectangle(b, 4.0f);
        }
    }
};

//==============================================================================
// MODEL AWARE KNOB
// Knows how to grey itself out
// Shows override label when model disables it
//==============================================================================
class ModelKnob : public juce::Component
{
public:
    ModelKnob()
    {
        slider.setSliderStyle(
            juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 65, 16);
        slider.setColour(
            juce::Slider::rotarySliderFillColourId,
            juce::Colour(0xFFE8A838));
        slider.setColour(
            juce::Slider::rotarySliderOutlineColourId,
            juce::Colour(0xFF2A2A2A));
        slider.setColour(
            juce::Slider::textBoxTextColourId,
            juce::Colour(0xFFE8C878));
        slider.setColour(
            juce::Slider::textBoxBackgroundColourId,
            juce::Colour(0xFF0A0A0A));
        slider.setColour(
            juce::Slider::textBoxOutlineColourId,
            juce::Colour(0xFF0A0A0A));
        addAndMakeVisible(slider);

        nameLabel.setJustificationType(
            juce::Justification::centred);
        nameLabel.setFont(juce::Font(
            juce::FontOptions(9.0f)));
        nameLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFFE8C878));
        addAndMakeVisible(nameLabel);

        overrideLabel.setJustificationType(
            juce::Justification::centred);
        overrideLabel.setFont(juce::Font(
            juce::FontOptions(9.0f).withStyle("Bold")));
        overrideLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFF666666));
        overrideLabel.setVisible(false);
        addAndMakeVisible(overrideLabel);
    }

    void setName(const juce::String& name)
    {
        nameLabel.setText(name,
            juce::dontSendNotification);
        currentName = name;
    }

    // Call this when model changes
    // enabled = false greys the knob out
    // overrideText = text shown instead of value
    void setModelState(bool enabled,
                       const juce::String& overrideText = "")
    {
        slider.setEnabled(enabled);
        slider.setAlpha(enabled ? 1.0f : 0.3f);

        if (!enabled && overrideText.isNotEmpty())
        {
            nameLabel.setText(overrideText,
                juce::dontSendNotification);
            nameLabel.setColour(
                juce::Label::textColourId,
                juce::Colour(0xFF555555));
        }
        else
        {
            nameLabel.setText(currentName,
                juce::dontSendNotification);
            nameLabel.setColour(
                juce::Label::textColourId,
                juce::Colour(0xFFE8C878));
        }
    }

    void resized() override
    {
        auto b = getLocalBounds();
        nameLabel.setBounds(b.removeFromTop(14));
        slider.setBounds(b);
    }

    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  overrideLabel;
    juce::String currentName;
};

//==============================================================================
// STEPPED COMBO
// Looks like hardware stepped selector
//==============================================================================
class SteppedCombo : public juce::ComboBox
{
public:
    SteppedCombo()
    {
        setColour(juce::ComboBox::backgroundColourId,
            juce::Colour(0xFF1A1A1A));
        setColour(juce::ComboBox::textColourId,
            juce::Colour(0xFFE8A838));
        setColour(juce::ComboBox::outlineColourId,
            juce::Colour(0xFF3A3A3A));
        setColour(juce::ComboBox::arrowColourId,
            juce::Colour(0xFFE8A838));
    }
};

//==============================================================================
// MAIN EDITOR
//==============================================================================
class ModulatedStripEditor
    : public juce::AudioProcessorEditor,
      public juce::Timer,
      public juce::ComboBox::Listener
{
public:
    ModulatedStripEditor(ModulatedStripProcessor&);
    ~ModulatedStripEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox*) override;

private:
    ModulatedStripProcessor& processor;

    // INPUT
    ModelKnob inputGainKnob;

    // SATURATION
    SteppedCombo satModelSelector;
    ModelKnob    driveKnob;
    ModelKnob    satMixKnob;
    BypassButton satBypassBtn { "ON" };

    // COMPRESSOR
    SteppedCombo compModelSelector;
    ModelKnob    thresholdKnob;
    ModelKnob    ratioKnob;
    ModelKnob    attackKnob;
    ModelKnob    releaseKnob;
    ModelKnob    makeupKnob;
    ModelKnob    compMixKnob;
    ModelKnob    kneeKnob;
    BypassButton compBypassBtn { "ON" };

    SteppedCombo fairchildTCSelector;
    BypassButton allInBtn      { "ALL IN" };
    BypassButton thrustBtn     { "THRUST" };
    BypassButton topologyBtn   { "FWD"    };
    BypassButton la2aLimitBtn  { "LIMIT"  };

    juce::Label compModelHintLabel;

    // EQ
    SteppedCombo eqModelSelector;
    ModelKnob    eqLowGainKnob;
    ModelKnob    eqLowFreqKnob;
    ModelKnob    eqMidGainKnob;
    ModelKnob    eqMidFreqKnob;
    ModelKnob    eqMidQKnob;
    ModelKnob    eqHighGainKnob;
    ModelKnob    eqHighFreqKnob;
    ModelKnob    eqHPFKnob;
    BypassButton eqBypassBtn  { "ON"     };
    BypassButton eqPreCompBtn { "EQ PRE" };

    juce::Label eqModelHintLabel;

    // OUTPUT
    ModelKnob outputGainKnob;

    // METERS
    VUMeter inputMeter;
    VUMeter outputMeter;
    VUMeter grMeter;
    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;
    juce::Label grMeterLabel;

    // ATTACHMENTS
    using SliderAtt = juce::AudioProcessorValueTreeState
        ::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState
        ::ButtonAttachment;

    std::unique_ptr<SliderAtt> inputGainAtt;
    std::unique_ptr<SliderAtt> driveAtt;
    std::unique_ptr<SliderAtt> satMixAtt;
    std::unique_ptr<ComboAtt>  satModelAtt;
    std::unique_ptr<ButtonAtt> satBypassAtt;

    std::unique_ptr<SliderAtt> thresholdAtt;
    std::unique_ptr<SliderAtt> ratioAtt;
    std::unique_ptr<SliderAtt> attackAtt;
    std::unique_ptr<SliderAtt> releaseAtt;
    std::unique_ptr<SliderAtt> makeupAtt;
    std::unique_ptr<SliderAtt> compMixAtt;
    std::unique_ptr<SliderAtt> kneeAtt;
    std::unique_ptr<ComboAtt>  compModelAtt;
    std::unique_ptr<ButtonAtt> compBypassAtt;
    std::unique_ptr<ComboAtt>  fairchildTCAtt;
	
	std::unique_ptr<ButtonAtt> allInAtt;
    std::unique_ptr<ButtonAtt> thrustAtt;
    std::unique_ptr<ButtonAtt> topologyAtt;
    std::unique_ptr<ButtonAtt> la2aLimitAtt;

    std::unique_ptr<SliderAtt> eqLowGainAtt;
    std::unique_ptr<SliderAtt> eqLowFreqAtt;
    std::unique_ptr<SliderAtt> eqMidGainAtt;
    std::unique_ptr<SliderAtt> eqMidFreqAtt;
    std::unique_ptr<SliderAtt> eqMidQAtt;
    std::unique_ptr<SliderAtt> eqHighGainAtt;
    std::unique_ptr<SliderAtt> eqHighFreqAtt;
    std::unique_ptr<SliderAtt> eqHPFAtt;
    std::unique_ptr<ComboAtt>  eqModelAtt;
    std::unique_ptr<ButtonAtt> eqBypassAtt;
    std::unique_ptr<ButtonAtt> eqPreCompAtt;

    std::unique_ptr<SliderAtt> outputGainAtt;

    void updateCompressorUI(int modelIndex);
    void updateEQUI(int modelIndex);
    void setupCombo(SteppedCombo& box,
                    const juce::StringArray& items);
    void setupMeterLabel(juce::Label& label,
                         const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};