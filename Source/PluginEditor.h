#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ModulatedStripEditor : public juce::AudioProcessorEditor
{
public:
    ModulatedStripEditor(ModulatedStripProcessor&);
    ~ModulatedStripEditor() override;

    // Draw the interface
    void paint(juce::Graphics&) override;
    
    // Position all elements
    void resized() override;

private:
    // Reference to our processor
    ModulatedStripProcessor& processor;

    // Input gain slider
    juce::Slider inputGainSlider;
    juce::Label  inputGainLabel;

    // Connects the slider to the parameter
    std::unique_ptr<juce::AudioProcessorValueTreeState
        ::SliderAttachment> inputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripEditor)
};