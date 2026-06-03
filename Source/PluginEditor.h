#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

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
        addAndMakeVisible(slider);

        nameLabel.setJustificationType(
            juce::Justification::centred);
        nameLabel.setFont(juce::Font(
            juce::FontOptions(9.0f)));
        nameLabel.setColour(
            juce::Label::textColourId,
            juce::Colour(0xFFE8C878));
        addAndMakeVisible(nameLabel);
    }

    void setName(const juce::String& name)
    {
        nameLabel.setText(name,
            juce::dontSendNotification);
        currentName = name;
    }

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
    juce::String currentName;
};

//==============================================================================
// STEPPED COMBO
//==============================================================================
class SteppedCombo : public juce::ComboBox
{
public:
    SteppedCombo() {}
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

    // Custom look and feel
    AnalogLookAndFeel analogLAF;

    //──────────────────────────────────────────────
    // INPUT SECTION
    //──────────────────────────────────────────────
    ModelKnob inputGainKnob;

    //──────────────────────────────────────────────
    // SATURATION SECTION
    //──────────────────────────────────────────────
    SteppedCombo satModelSelector;
    ModelKnob    driveKnob;
    ModelKnob    satMixKnob;
    juce::ToggleButton satBypassBtn { "ON" };

    //──────────────────────────────────────────────
    // COMPRESSOR SECTION
    //──────────────────────────────────────────────
    SteppedCombo compModelSelector;
    ModelKnob    thresholdKnob;
    ModelKnob    ratioKnob;
    ModelKnob    attackKnob;
    ModelKnob    releaseKnob;
    ModelKnob    makeupKnob;
    ModelKnob    compMixKnob;
    ModelKnob    kneeKnob;
    juce::ToggleButton compBypassBtn { "ON" };

    // Model-specific compressor controls
    SteppedCombo       fairchildTCSelector;
    juce::ToggleButton allInBtn      { "ALL IN"  };
    juce::ToggleButton thrustBtn     { "THRUST"  };
    juce::ToggleButton topologyBtn   { "FWD"     };
    juce::ToggleButton la2aLimitBtn  { "LIMIT"   };

    juce::Label compModelHintLabel;

    // Analog needle GR meter
    AnalogNeedleMeter grNeedleMeter;

    //──────────────────────────────────────────────
    // EQ SECTION
    //──────────────────────────────────────────────
    SteppedCombo eqModelSelector;
    ModelKnob    eqLowGainKnob;
    ModelKnob    eqLowFreqKnob;
    ModelKnob    eqMidGainKnob;
    ModelKnob    eqMidFreqKnob;
    ModelKnob    eqMidQKnob;
    ModelKnob    eqHighGainKnob;
    ModelKnob    eqHighFreqKnob;
    ModelKnob    eqHPFKnob;
    juce::ToggleButton eqBypassBtn  { "ON"      };
    juce::ToggleButton eqPreCompBtn { "EQ PRE"  };

    juce::Label eqModelHintLabel;

    //──────────────────────────────────────────────
    // OUTPUT SECTION
    //──────────────────────────────────────────────
    ModelKnob outputGainKnob;

    //──────────────────────────────────────────────
    // METERS
    //──────────────────────────────────────────────
    LEDLadderMeter inputMeter;
    LEDLadderMeter outputMeter;

    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;

    //──────────────────────────────────────────────
    // SCREWS
    //──────────────────────────────────────────────
    HardwareScrew screwTL, screwTR, screwBL, screwBR;

    //──────────────────────────────────────────────
    // PARAMETER ATTACHMENTS
    //──────────────────────────────────────────────
    using SliderAtt = juce::AudioProcessorValueTreeState
        ::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState
        ::ButtonAttachment;

    // Input
    std::unique_ptr<SliderAtt> inputGainAtt;

    // Saturation
    std::unique_ptr<SliderAtt> driveAtt;
    std::unique_ptr<SliderAtt> satMixAtt;
    std::unique_ptr<ComboAtt>  satModelAtt;
    std::unique_ptr<ButtonAtt> satBypassAtt;

    // Compressor
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

    // EQ
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

    // Output
    std::unique_ptr<SliderAtt> outputGainAtt;

    //──────────────────────────────────────────────
    // UI STATE MACHINE
    //──────────────────────────────────────────────
    void updateCompressorUI(int modelIndex);
    void updateEQUI(int modelIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};