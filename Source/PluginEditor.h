#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ModulatedStripEditor : public juce::AudioProcessorEditor
{
public:
    ModulatedStripEditor(ModulatedStripProcessor&);
    ~ModulatedStripEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ModulatedStripProcessor& processor;

    // INPUT
    juce::Slider inputGainSlider;
    juce::Label  inputGainLabel;

    // SATURATION
    juce::ComboBox satModelSelector;
    juce::Slider driveSlider;
    juce::Label  driveLabel;
    juce::Slider satMixSlider;
    juce::Label  satMixLabel;

    // COMPRESSOR
    juce::ComboBox compModelSelector;
    juce::Slider thresholdSlider;
    juce::Label  thresholdLabel;
    juce::Slider ratioSlider;
    juce::Label  ratioLabel;
    juce::Slider attackSlider;
    juce::Label  attackLabel;
    juce::Slider releaseSlider;
    juce::Label  releaseLabel;
    juce::Slider makeupSlider;
    juce::Label  makeupLabel;
    juce::Slider compMixSlider;
    juce::Label  compMixLabel;
    juce::Slider kneeSlider;
    juce::Label  kneeLabel;

    // EQ
    juce::ComboBox eqModelSelector;
    juce::Slider eqLowGainSlider;
    juce::Label  eqLowGainLabel;
    juce::Slider eqLowFreqSlider;
    juce::Label  eqLowFreqLabel;
    juce::Slider eqMidGainSlider;
    juce::Label  eqMidGainLabel;
    juce::Slider eqMidFreqSlider;
    juce::Label  eqMidFreqLabel;
    juce::Slider eqMidQSlider;
    juce::Label  eqMidQLabel;
    juce::Slider eqHighGainSlider;
    juce::Label  eqHighGainLabel;
    juce::Slider eqHighFreqSlider;
    juce::Label  eqHighFreqLabel;
    juce::Slider eqHPFSlider;
    juce::Label  eqHPFLabel;

    // OUTPUT
    juce::Slider outputGainSlider;
    juce::Label  outputGainLabel;

    // ATTACHMENTS - Input
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> inputGainAttachment;

    // ATTACHMENTS - Saturation
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> satMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> satModelAttachment;

    // ATTACHMENTS - Compressor
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

    // ATTACHMENTS - EQ
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

    // OUTPUT
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> outputGainAttachment;

    void setupKnob(juce::Slider& slider,
                   juce::Label& label,
                   const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};