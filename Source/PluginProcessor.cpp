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

    // Get all parameter values
    float gainDb  = apvts.getRawParameterValue(
                        "inputGain")->load();
    float drive   = apvts.getRawParameterValue(
                        "drive")->load() / 100.0f;
    float satMix  = apvts.getRawParameterValue(
                        "satMix")->load() / 100.0f;
    int   model   = static_cast<int>(
                        apvts.getRawParameterValue(
                        "satModel")->load());

    // Apply input gain first
    float gain = juce::Decibels::decibelsToGain(gainDb);
    buffer.applyGain(gain);

    // Apply saturation
    saturation.process(buffer, drive, satMix, model);
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

    // INPUT GAIN
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "inputGain",
            "Input Gain",
            -24.0f,
            24.0f,
            0.0f
        )
    );

    // SATURATION DRIVE
    // 0 = no saturation, 100 = maximum saturation
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "drive",
            "Drive",
            0.0f,
            100.0f,
            0.0f
        )
    );

    // SATURATION MIX (parallel blend)
    // 0 = dry only, 100 = wet only
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "satMix",
            "Sat Mix",
            0.0f,
            100.0f,
            100.0f
        )
    );

    // SATURATION MODEL SELECTOR
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "satModel",
            "Sat Model",
            juce::StringArray{
                "NEVE",
                "SSL",
                "API",
                "TUBE",
                "TAPE",
                "FET",
                "IRON"
            },
            0  // default = NEVE
        )
    );

    return { params.begin(), params.end() };
}

// This function creates the plugin instance
// The VST system calls this when loading the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}