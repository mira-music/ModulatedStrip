#include "PluginProcessor.h"
#include "PluginEditor.h"

ModulatedStripProcessor::ModulatedStripProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput ("Input",
                juce::AudioChannelSet::stereo(), true)
            .withOutput("Output",
                juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters())
{
    //──────────────────────────────────────────────
    // P1 FIX - Cache all parameter pointers here
    // This runs once at plugin load
    // processBlock reads cached pointers directly
    // No string map lookups on audio thread
    //──────────────────────────────────────────────
    pInputGain     = apvts.getRawParameterValue("inputGain");
    pOutputGain    = apvts.getRawParameterValue("outputGain");
    pDrive         = apvts.getRawParameterValue("drive");
    pSatMix        = apvts.getRawParameterValue("satMix");
    pSatModel      = apvts.getRawParameterValue("satModel");
    pSatBypass     = apvts.getRawParameterValue("satBypass");
    pCompModel     = apvts.getRawParameterValue("compModel");
    pCompThreshold = apvts.getRawParameterValue("compThreshold");
    pCompRatio     = apvts.getRawParameterValue("compRatio");
    pCompAttack    = apvts.getRawParameterValue("compAttack");
    pCompRelease   = apvts.getRawParameterValue("compRelease");
    pCompMakeup    = apvts.getRawParameterValue("compMakeup");
    pCompMix       = apvts.getRawParameterValue("compMix");
    pCompKnee      = apvts.getRawParameterValue("compKnee");
    pCompBypass    = apvts.getRawParameterValue("compBypass");
    pEqModel       = apvts.getRawParameterValue("eqModel");
    pEqLowGain     = apvts.getRawParameterValue("eqLowGain");
    pEqLowFreq     = apvts.getRawParameterValue("eqLowFreq");
    pEqMidGain     = apvts.getRawParameterValue("eqMidGain");
    pEqMidFreq     = apvts.getRawParameterValue("eqMidFreq");
    pEqMidQ        = apvts.getRawParameterValue("eqMidQ");
    pEqHighGain    = apvts.getRawParameterValue("eqHighGain");
    pEqHighFreq    = apvts.getRawParameterValue("eqHighFreq");
    pEqHPF         = apvts.getRawParameterValue("eqHPF");
    pEqBypass      = apvts.getRawParameterValue("eqBypass");
    pEqPreComp     = apvts.getRawParameterValue("eqPreComp");
}

ModulatedStripProcessor::~ModulatedStripProcessor() {}

void ModulatedStripProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    saturation.prepare(sampleRate);
    compressor.prepare(sampleRate);
    equalizer.prepare(sampleRate);

    //──────────────────────────────────────────────
    // P1 FIX - Initialize smoothed values
    // Time in seconds for parameter to ramp
    // 10ms for gain - 20ms for drive and freq
    //──────────────────────────────────────────────
    inputGainSmoothed .reset(sampleRate, 0.01);
    outputGainSmoothed.reset(sampleRate, 0.01);
    driveSmoothed     .reset(sampleRate, 0.02);
    makeupSmoothed    .reset(sampleRate, 0.01);

    inputGainSmoothed .setCurrentAndTargetValue(1.0f);
    outputGainSmoothed.setCurrentAndTargetValue(1.0f);
    driveSmoothed     .setCurrentAndTargetValue(0.0f);
    makeupSmoothed    .setCurrentAndTargetValue(1.0f);

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

    int numSamples  = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    //──────────────────────────────────────────────
    // READ ALL PARAMETERS
    // Using cached pointers - no string lookups
    //──────────────────────────────────────────────
    float inputGainDb  = pInputGain->load();
    float outputGainDb = pOutputGain->load();
    float drive        = pDrive->load()   / 100.0f;
    float satMix       = pSatMix->load()  / 100.0f;
    int   satModel     = static_cast<int>(pSatModel->load());
    bool  satBypassed  = pSatBypass->load()  > 0.5f;

    int   compModel    = static_cast<int>(pCompModel->load());
    float compThresh   = pCompThreshold->load();
    float compRatio    = pCompRatio->load();
    float compAttack   = pCompAttack->load();
    float compRelease  = pCompRelease->load();
    float compMakeup   = pCompMakeup->load();
    float compMix      = pCompMix->load()  / 100.0f;
    float compKnee     = pCompKnee->load();
    bool  compBypassed = pCompBypass->load() > 0.5f;

    int   eqModel      = static_cast<int>(pEqModel->load());
    float eqLowGain    = pEqLowGain->load();
    float eqLowFreq    = pEqLowFreq->load();
    float eqMidGain    = pEqMidGain->load();
    float eqMidFreq    = pEqMidFreq->load();
    float eqMidQ       = pEqMidQ->load();
    float eqHighGain   = pEqHighGain->load();
    float eqHighFreq   = pEqHighFreq->load();
    float eqHPF        = pEqHPF->load();
    bool  eqBypassed   = pEqBypass->load()  > 0.5f;
    bool  eqPreComp    = pEqPreComp->load()  > 0.5f;

    //──────────────────────────────────────────────
    // UPDATE SMOOTHED VALUES
    // These ramp gradually to target
    // Preventing zipper noise and clicks
    //──────────────────────────────────────────────
    inputGainSmoothed .setTargetValue(
        juce::Decibels::decibelsToGain(inputGainDb));
    outputGainSmoothed.setTargetValue(
        juce::Decibels::decibelsToGain(outputGainDb));
    driveSmoothed     .setTargetValue(drive);
    makeupSmoothed    .setTargetValue(
        juce::Decibels::decibelsToGain(compMakeup));

    //──────────────────────────────────────────────
    // UPDATE EQ PARAMETERS
    // EQ uses dirty flag internally
    // Only recalculates when something changed
    //──────────────────────────────────────────────
    equalizer.setModel    (eqModel);
    equalizer.setLowGain  (eqLowGain);
    equalizer.setLowFreq  (eqLowFreq);
    equalizer.setMidGain  (eqMidGain);
    equalizer.setMidFreq  (eqMidFreq);
    equalizer.setMidQ     (eqMidQ);
    equalizer.setHighGain (eqHighGain);
    equalizer.setHighFreq (eqHighFreq);
    equalizer.setHPF      (eqHPF);

    //──────────────────────────────────────────────
    // STAGE 1 - INPUT GAIN (smoothed)
    //──────────────────────────────────────────────
    // Measure input peak before processing
    float inPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ch++)
        inPeak = std::max(inPeak,
            buffer.getMagnitude(ch, 0, numSamples));
    inputPeak.store(inPeak);

    // Apply smoothed input gain sample by sample
    for (int i = 0; i < numSamples; i++)
    {
        float g = inputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }
    // Reset smoothed value back to start of block
    // so EQ pre/post ordering can use same smoothing
    inputGainSmoothed.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(inputGainDb));

    //──────────────────────────────────────────────
    // STAGE 2 - SATURATION
    //──────────────────────────────────────────────
    if (!satBypassed)
    {
        float currentDrive = driveSmoothed.getNextValue();
        driveSmoothed.setCurrentAndTargetValue(
            currentDrive);
        saturation.process(buffer, currentDrive,
                           satMix, satModel);
    }

    //──────────────────────────────────────────────
    // STAGE 3 & 4 - EQ AND COMPRESSOR
    // Order depends on eqPreComp switch
    // eqPreComp = true  → EQ before compressor
    // eqPreComp = false → compressor before EQ
    //──────────────────────────────────────────────
    if (eqPreComp)
    {
        // EQ → COMP
        if (!eqBypassed)
            equalizer.process(buffer);

        if (!compBypassed)
        {
            compressor.setModel    (compModel);
            compressor.setThreshold(compThresh);
            compressor.setRatio    (compRatio);
            compressor.setAttack   (compAttack);
            compressor.setRelease  (compRelease);
            compressor.setMakeupSmoothed(makeupSmoothed);
            compressor.setMix      (compMix);
            compressor.setKnee     (compKnee);
            compressor.process     (buffer);
        }
    }
    else
    {
        // COMP → EQ (default analog desk order)
        if (!compBypassed)
        {
            compressor.setModel    (compModel);
            compressor.setThreshold(compThresh);
            compressor.setRatio    (compRatio);
            compressor.setAttack   (compAttack);
            compressor.setRelease  (compRelease);
            compressor.setMakeupSmoothed(makeupSmoothed);
            compressor.setMix      (compMix);
            compressor.setKnee     (compKnee);
            compressor.process     (buffer);
        }

        if (!eqBypassed)
            equalizer.process(buffer);
    }

    //──────────────────────────────────────────────
    // STAGE 5 - OUTPUT GAIN (smoothed)
    //──────────────────────────────────────────────
    outputGainSmoothed.setTargetValue(
        juce::Decibels::decibelsToGain(outputGainDb));

    for (int i = 0; i < numSamples; i++)
    {
        float g = outputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }

    // Measure output peak for meter
    float outPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ch++)
        outPeak = std::max(outPeak,
            buffer.getMagnitude(ch, 0, numSamples));
    outputPeak.store(outPeak);
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

    // INPUT / OUTPUT
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "inputGain",  "Input Gain",
            -24.0f, 24.0f, 0.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "outputGain", "Output Gain",
            -24.0f, 24.0f, 0.0f));

    // SATURATION
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "drive",    "Drive",
            0.0f, 100.0f, 0.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "satMix",   "Sat Mix",
            0.0f, 100.0f, 100.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "satModel", "Sat Model",
            juce::StringArray{
                "NEVE","SSL","API",
                "TUBE","TAPE","FET","IRON"},
            0));
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "satBypass", "Sat Bypass", false));

    // COMPRESSOR
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "compModel", "Comp Model",
            juce::StringArray{
                "SSL Bus","Fairchild 670",
                "LA-2A","1176","API 2500"},
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
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "compBypass", "Comp Bypass", false));

    // EQ
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "eqModel", "EQ Model",
            juce::StringArray{
                "Neve 1073","Neve 1084",
                "SSL 4000E","Pultec EQP-1A",
                "API 550A"},
            0));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqLowGain",  "Low Gain",
            -15.0f, 15.0f, 0.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqLowFreq",  "Low Freq",
            30.0f, 300.0f, 100.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidGain",  "Mid Gain",
            -15.0f, 15.0f, 0.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidFreq",  "Mid Freq",
            200.0f, 8000.0f, 1000.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "eqMidQ",     "Mid Q",
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
            "eqHPF",      "HPF",
            20.0f, 500.0f, 20.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "eqBypass",   "EQ Bypass",  false));
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "eqPreComp",  "EQ Pre Comp", false));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}