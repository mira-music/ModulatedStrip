#include "PluginProcessor.h"
#include "PluginEditor.h"

// Constructor - runs when plugin loads
ModulatedStripProcessor::ModulatedStripProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput ("Input",  
                        juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", 
                        juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters())
{
}

ModulatedStripProcessor::~ModulatedStripProcessor() {}

// Called when Ableton presses play
void ModulatedStripProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    (void)sampleRate;
    (void)samplesPerBlock;
}

void ModulatedStripProcessor::releaseResources() {}

// This creates the visual interface
juce::AudioProcessorEditor* 
ModulatedStripProcessor::createEditor()
{
    return new ModulatedStripEditor(*this);
}

// THIS IS WHERE AUDIO PROCESSING HAPPENS
void ModulatedStripProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto inputGain = apvts.getRawParameterValue("inputGain");
    float gainDb   = inputGain->load();
    float gain = juce::Decibels::decibelsToGain(gainDb);

    for (int channel = 0; 
         channel < buffer.getNumChannels(); 
         channel++)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; 
             sample < buffer.getNumSamples(); 
             sample++)
        {
            channelData[sample] *= gain;
        }
    }
}

// Save plugin state
void ModulatedStripProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

// Load plugin state
void ModulatedStripProcessor::setStateInformation(
    const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState && 
        xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(
            juce::ValueTree::fromXml(*xmlState));
    }
}

// Define all parameters
juce::AudioProcessorValueTreeState::ParameterLayout
ModulatedStripProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> 
        params;

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "inputGain",
            "Input Gain",
            -24.0f,
            24.0f,
            0.0f
        )
    );

    return { params.begin(), params.end() };
}

// This creates the plugin instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}