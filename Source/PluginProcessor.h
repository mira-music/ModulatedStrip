#pragma once
#include <JuceHeader.h>
#include "SaturationProcessor.h"

class ModulatedStripProcessor : public juce::AudioProcessor
{
public:
    ModulatedStripProcessor();
    ~ModulatedStripProcessor() override;

    // Ableton calls this to set up
    void prepareToPlay(double sampleRate, 
                       int samplesPerBlock) override;
    
    void releaseResources() override;

    // Ableton calls this for every block of audio
    void processBlock(juce::AudioBuffer<float>&, 
                      juce::MidiBuffer&) override;

    // Editor is the visual interface
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // Plugin info
    const juce::String getName() const override 
    { return "Modulated Strip"; }
    
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Programs (presets - we add these later)
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    // Save and load settings
    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, 
                             int sizeInBytes) override;

    // Parameter tree (all our knobs live here)
    juce::AudioProcessorValueTreeState apvts;

private:
    // Creates all our parameters (knobs, buttons)
    juce::AudioProcessorValueTreeState::ParameterLayout
        createParameters();

	// Our saturation engine
    SaturationProcessor saturation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripProcessor)
};