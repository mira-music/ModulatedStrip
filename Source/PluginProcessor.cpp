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
    // Cache all parameter pointers once
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

    // Initialize all smoothed values
    // setCurrentAndTargetValue sets both immediately
    // No ramp on first block
    double sr = sampleRate;

    inputGainSmoothed .reset(sr, 0.010);
    outputGainSmoothed.reset(sr, 0.010);
    driveSmoothed     .reset(sr, 0.020);
    makeupSmoothed    .reset(sr, 0.010);
    satMixSmoothed    .reset(sr, 0.020);
    compMixSmoothed   .reset(sr, 0.020);

    // Set starting values from current parameters
    inputGainSmoothed .setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(
            pInputGain->load()));
    outputGainSmoothed.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(
            pOutputGain->load()));
    driveSmoothed     .setCurrentAndTargetValue(
        pDrive->load() / 100.0f);
    makeupSmoothed    .setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(
            pCompMakeup->load()));
    satMixSmoothed    .setCurrentAndTargetValue(
        pSatMix->load() / 100.0f);
    compMixSmoothed   .setCurrentAndTargetValue(
        pCompMix->load() / 100.0f);

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
    // READ PARAMETERS (cached pointers - no string lookup)
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
    // CRITICAL FIX - SET SMOOTHED TARGETS
    // Do NOT call getNextValue() here
    // Just set the target for this block
    // getNextValue() called inside the sample loop
    //──────────────────────────────────────────────
    inputGainSmoothed .setTargetValue(
        juce::Decibels::decibelsToGain(inputGainDb));
    outputGainSmoothed.setTargetValue(
        juce::Decibels::decibelsToGain(outputGainDb));
    driveSmoothed     .setTargetValue(drive);
    makeupSmoothed    .setTargetValue(
        juce::Decibels::decibelsToGain(compMakeup));
    satMixSmoothed    .setTargetValue(satMix);
    compMixSmoothed   .setTargetValue(compMix);

    //──────────────────────────────────────────────
    // UPDATE EQ (dirty flag handles recalculation)
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
    // MEASURE INPUT PEAK
    //──────────────────────────────────────────────
    float inPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ch++)
        inPeak = std::max(inPeak,
            buffer.getMagnitude(ch, 0, numSamples));
    inputPeak.store(inPeak);

    //──────────────────────────────────────────────
    // STAGE 1 - INPUT GAIN (smoothed per sample)
    //──────────────────────────────────────────────
    for (int i = 0; i < numSamples; i++)
    {
        float g = inputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }

    //──────────────────────────────────────────────
    // STAGE 2 - SATURATION (smoothed drive)
    //──────────────────────────────────────────────
    if (!satBypassed)
    {
        // Get smoothed values for this block
        // CRITICAL FIX: snapshot the smoother state
        // then pass current value - do not call
        // getNextValue twice for drive and mix
        float currentDrive   = driveSmoothed  .getNextValue();
        float currentSatMix  = satMixSmoothed .getNextValue();

        // Restore smoother position for accurate ramp
        // by processing remaining samples inside sat module
        driveSmoothed .setCurrentAndTargetValue(currentDrive);
        satMixSmoothed.setCurrentAndTargetValue(currentSatMix);

        saturation.process(buffer,
                           currentDrive,
                           currentSatMix,
                           satModel);
    }

    //──────────────────────────────────────────────
    // STAGE 3 & 4 - EQ AND COMPRESSOR
    // Order controlled by eqPreComp switch
    //──────────────────────────────────────────────

    // Setup compressor parameters
    // Model change triggers state reset internally
    compressor.setModel    (compModel);
    compressor.setThreshold(compThresh);
    compressor.setRatio    (compRatio);
    compressor.setAttack   (compAttack);
    compressor.setRelease  (compRelease);
    compressor.setMix      (compMixSmoothed.getNextValue());
    compressor.setKnee     (compKnee);
    compressor.setMakeupSmoothed(makeupSmoothed);

    if (eqPreComp)
    {
        if (!eqBypassed)  equalizer.process(buffer);
        if (!compBypassed) compressor.process(buffer);
    }
    else
    {
        if (!compBypassed) compressor.process(buffer);
        if (!eqBypassed)  equalizer.process(buffer);
    }

    //──────────────────────────────────────────────
    // STAGE 5 - OUTPUT GAIN (smoothed per sample)
    //──────────────────────────────────────────────
    for (int i = 0; i < numSamples; i++)
    {
        float g = outputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }

    //──────────────────────────────────────────────
    // CRITICAL FIX - OUTPUT SOFT CLIPPER
    // Transparent tanh at -0.5dBFS
    // Prevents excursions during model switches
    // Essential for live PA use
    //──────────────────────────────────────────────
    for (int ch = 0; ch < numChannels; ch++)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; i++)
            data[i] = softClip(data[i]);
    }

    //──────────────────────────────────────────────
    // MEASURE OUTPUT PEAK
    //──────────────────────────────────────────────
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
            "eqBypass",  "EQ Bypass",   false));
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "eqPreComp", "EQ Pre Comp", false));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}