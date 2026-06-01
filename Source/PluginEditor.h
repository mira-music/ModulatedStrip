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

    // ATTACHMENTS
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> satMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> satModelAttachment;
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

    void setupKnob(juce::Slider& slider,
                   juce::Label& label,
                   const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};