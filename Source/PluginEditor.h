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

    // INPUT SECTION
    juce::Slider inputGainSlider;
    juce::Label  inputGainLabel;

    // SATURATION SECTION
    juce::Slider driveSlider;
    juce::Label  driveLabel;

    juce::Slider satMixSlider;
    juce::Label  satMixLabel;

    juce::ComboBox satModelSelector;
    juce::Label    satModelLabel;

    // Parameter attachments
    // These sync the UI controls to the parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> satMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::ComboBoxAttachment> satModelAttachment;

    // Helper to set up a knob consistently
    void setupKnob(juce::Slider& slider, 
                   juce::Label& label,
                   const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};