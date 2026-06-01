#include "PluginProcessor.h"
#include "PluginEditor.h"

ModulatedStripProcessor::ModulatedStripProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input",  
                juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", 
                juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters())
{
}

ModulatedStripProcessor::~ModulatedStripProcessor() {}

void ModulatedStripProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    saturation.prepare(sampleRate);
    compressor.prepare(sampleRate);
    (void)samplesPerBlock;
}

void ModulatedStripProcessor::releaseResources() {}

juce::AudioProcessorEditor* 
ModulatedStripProcessor::createEditor()
{
    return new ModulatedStripEditor(*this);
}

void ModulatedStripProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, 
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Input
    float gainDb = apvts.getRawParameterValue(
        "inputGain")->load();

    // Saturation
    float drive = apvts.getRawParameterValue(
        "drive")->load() / 100.0f;
    float satMix = apvts.getRawParameterValue(
        "satMix")->load() / 100.0f;
    int satModel = static_cast<int>(
        apvts.getRawParameterValue("satModel")->load());

    // Compressor
    int compModel = static_cast<int>(
        apvts.getRawParameterValue("compModel")->load());
    float compThresh = apvts.getRawParameterValue(
        "compThreshold")->load();
    float compRatio = apvts.getRawParameterValue(
        "compRatio")->load();
    float compAttack = apvts.getRawParameterValue(
        "compAttack")->load();
    float compRelease = apvts.getRawParameterValue(
        "compRelease")->load();
    float compMakeup = apvts.getRawParameterValue(
        "compMakeup")->load();
    float compMix = apvts.getRawParameterValue(
        "compMix")->load() / 100.0f;
    float compKnee = apvts.getRawParameterValue(
        "compKnee")->load();

    // STAGE 1 - Input Gain
    float gain = juce::Decibels::decibelsToGain(gainDb);
    buffer.applyGain(gain);

    // STAGE 2 - Saturation
    saturation.process(buffer, drive, satMix, satModel);

    // STAGE 3 - Compressor
    compressor.setModel(compModel);
    compressor.setThreshold(compThresh);
    compressor.setRatio(compRatio);
    compressor.setAttack(compAttack);
    compressor.setRelease(compRelease);
    compressor.setMakeup(compMakeup);
    compressor.setMix(compMix);
    compressor.setKnee(compKnee);
    compressor.process(buffer);
}

void ModulatedStripProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(
        state.createXml());
    copyXmlToBinary(*xml, destData);
}

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

juce::AudioProcessorValueTreeState::ParameterLayout
ModulatedStripProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> 
        params;

    // INPUT
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "inputGain", "Input Gain",
            -24.0f, 24.0f, 0.0f));

    // SATURATION
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "drive", "Drive",
            0.0f, 100.0f, 0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "satMix", "Sat Mix",
            0.0f, 100.0f, 100.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "satModel", "Sat Model",
            juce::StringArray{
                "NEVE", "SSL", "API", "TUBE", 
                "TAPE", "FET", "IRON"},
            0));

    // COMPRESSOR
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "compModel", "Comp Model",
            juce::StringArray{
                "SSL Bus", "Fairchild 670", "LA-2A", 
                "1176", "API 2500"},
            0));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compThreshold", "Threshold",
            -60.0f, 0.0f, -20.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compRatio", "Ratio",
            1.0f, 20.0f, 4.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compAttack", "Attack",
            0.1f, 100.0f, 10.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compRelease", "Release",
            10.0f, 2000.0f, 100.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compMakeup", "Makeup",
            0.0f, 24.0f, 0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compMix", "Comp Mix",
            0.0f, 100.0f, 100.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "compKnee", "Knee",
            0.0f, 12.0f, 6.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}