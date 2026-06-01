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
    equalizer.prepare(sampleRate);
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

    //──────────────────────────────────────────
    // INPUT PARAMETERS
    //──────────────────────────────────────────
    float gainDb = apvts.getRawParameterValue(
        "inputGain")->load();

    //──────────────────────────────────────────
    // SATURATION PARAMETERS
    //──────────────────────────────────────────
    float drive = apvts.getRawParameterValue(
        "drive")->load() / 100.0f;
    float satMix = apvts.getRawParameterValue(
        "satMix")->load() / 100.0f;
    int satModel = static_cast<int>(
        apvts.getRawParameterValue("satModel")->load());

    //──────────────────────────────────────────
    // COMPRESSOR PARAMETERS
    //──────────────────────────────────────────
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

    //──────────────────────────────────────────
    // EQ PARAMETERS
    //──────────────────────────────────────────
    int eqModel = static_cast<int>(
        apvts.getRawParameterValue("eqModel")->load());
    float eqLowGain = apvts.getRawParameterValue(
        "eqLowGain")->load();
    float eqLowFreq = apvts.getRawParameterValue(
        "eqLowFreq")->load();
    float eqMidGain = apvts.getRawParameterValue(
        "eqMidGain")->load();
    float eqMidFreq = apvts.getRawParameterValue(
        "eqMidFreq")->load();
    float eqMidQ = apvts.getRawParameterValue(
        "eqMidQ")->load();
    float eqHighGain = apvts.getRawParameterValue(
        "eqHighGain")->load();
    float eqHighFreq = apvts.getRawParameterValue(
        "eqHighFreq")->load();
    float eqHPF = apvts.getRawParameterValue(
        "eqHPF")->load();
    float outputGain = apvts.getRawParameterValue(
        "outputGain")->load();

    //──────────────────────────────────────────
    // SIGNAL CHAIN
    // INPUT → SATURATION → COMPRESSOR → EQ → OUTPUT
    //──────────────────────────────────────────

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

    // STAGE 4 - EQ
    equalizer.setModel(eqModel);
    equalizer.setLowGain(eqLowGain);
    equalizer.setLowFreq(eqLowFreq);
    equalizer.setMidGain(eqMidGain);
    equalizer.setMidFreq(eqMidFreq);
    equalizer.setMidQ(eqMidQ);
    equalizer.setHighGain(eqHighGain);
    equalizer.setHighFreq(eqHighFreq);
    equalizer.setHPF(eqHPF);
    equalizer.process(buffer);

    // STAGE 5 - Output Gain
    float outGain = juce::Decibels::decibelsToGain(
        outputGain);
    buffer.applyGain(outGain);
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

    // EQ
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "eqModel", "EQ Model",
            juce::StringArray{
                "Neve 1073", "Neve 1084", "SSL 4000E", 
                "Pultec EQP-1A", "API 550A"},
            0));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqLowGain", "Low Gain",
            -15.0f, 15.0f, 0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqLowFreq", "Low Freq",
            30.0f, 300.0f, 100.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidGain", "Mid Gain",
            -15.0f, 15.0f, 0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidFreq", "Mid Freq",
            200.0f, 8000.0f, 1000.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidQ", "Mid Q",
            0.1f, 10.0f, 1.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqHighGain", "High Gain",
            -15.0f, 15.0f, 0.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqHighFreq", "High Freq",
            1500.0f, 16000.0f, 10000.0f));

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqHPF", "HPF",
            20.0f, 500.0f, 20.0f));

    // OUTPUT
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "outputGain", "Output Gain",
            -24.0f, 24.0f, 0.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}