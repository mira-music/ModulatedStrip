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
    // No string lookups on audio thread

    // Input / Output
    pInputGain     = apvts.getRawParameterValue("inputGain");
    pOutputGain    = apvts.getRawParameterValue("outputGain");

    // Saturation
    pDrive         = apvts.getRawParameterValue("drive");
    pSatMix        = apvts.getRawParameterValue("satMix");
    pSatModel      = apvts.getRawParameterValue("satModel");
    pSatBypass     = apvts.getRawParameterValue("satBypass");

    // Compressor
    pCompModel     = apvts.getRawParameterValue("compModel");
    pCompThreshold = apvts.getRawParameterValue("compThreshold");
    pCompRatio     = apvts.getRawParameterValue("compRatio");
    pCompAttack    = apvts.getRawParameterValue("compAttack");
    pCompRelease   = apvts.getRawParameterValue("compRelease");
    pCompMakeup    = apvts.getRawParameterValue("compMakeup");
    pCompMix       = apvts.getRawParameterValue("compMix");
    pCompKnee      = apvts.getRawParameterValue("compKnee");
    pCompBypass    = apvts.getRawParameterValue("compBypass");
    pFairchildTC   = apvts.getRawParameterValue("fairchildTC");

    // Compressor extra controls
    pAllButtonsIn  = apvts.getRawParameterValue("allButtonsIn");
    pThrustOn      = apvts.getRawParameterValue("thrustOn");
    pFeedbackMode  = apvts.getRawParameterValue("feedbackMode");
    pLa2aLimit     = apvts.getRawParameterValue("la2aLimit");

    // EQ
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
    equalizer .prepare(sampleRate);

    double sr = sampleRate;

    inputGainSmoothed .reset(sr, 0.010);
    outputGainSmoothed.reset(sr, 0.010);
    driveSmoothed     .reset(sr, 0.020);
    makeupSmoothed    .reset(sr, 0.010);
    satMixSmoothed    .reset(sr, 0.020);
    compMixSmoothed   .reset(sr, 0.020);

    // setCurrentAndTargetValue ONLY in prepareToPlay
    // Never in processBlock - that resets ramps mid-flight
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
    // READ ALL PARAMETERS
    //──────────────────────────────────────────────

    // Input / Output
    float inputGainDb  = pInputGain->load();
    float outputGainDb = pOutputGain->load();

    // Saturation
    int   satModel     = static_cast<int>(
                             pSatModel->load());
    bool  satBypassed  = pSatBypass->load() > 0.5f;

    // Compressor
    int   compModel    = static_cast<int>(
                             pCompModel->load());
    float compThresh   = pCompThreshold->load();
    float compRatio    = pCompRatio->load();
    float compAttack   = pCompAttack->load();
    float compRelease  = pCompRelease->load();
    float compKnee     = pCompKnee->load();
    bool  compBypassed = pCompBypass->load() > 0.5f;
    int   fairchildTC  = static_cast<int>(
                             pFairchildTC->load());

    // Compressor extra controls
    bool  allButtonsIn = pAllButtonsIn->load() > 0.5f;
    bool  thrustOn     = pThrustOn->load()     > 0.5f;
    bool  feedbackMode = pFeedbackMode->load() > 0.5f;
    bool  la2aLimit    = pLa2aLimit->load()    > 0.5f;

    // EQ
    int   eqModel      = static_cast<int>(
                             pEqModel->load());
    float eqLowGain    = pEqLowGain->load();
    float eqLowFreq    = pEqLowFreq->load();
    float eqMidGain    = pEqMidGain->load();
    float eqMidFreq    = pEqMidFreq->load();
    float eqMidQ       = pEqMidQ->load();
    float eqHighGain   = pEqHighGain->load();
    float eqHighFreq   = pEqHighFreq->load();
    float eqHPF        = pEqHPF->load();
    bool  eqBypassed   = pEqBypass->load()  > 0.5f;
    bool  eqPreComp    = pEqPreComp->load() > 0.5f;

    //──────────────────────────────────────────────
    // SET SMOOTHED TARGETS ONLY
    // Never setCurrentAndTargetValue in processBlock
    // That would reset ramps mid-flight
    //──────────────────────────────────────────────
    inputGainSmoothed .setTargetValue(
        juce::Decibels::decibelsToGain(inputGainDb));
    outputGainSmoothed.setTargetValue(
        juce::Decibels::decibelsToGain(outputGainDb));
    driveSmoothed     .setTargetValue(
        pDrive->load() / 100.0f);
    makeupSmoothed    .setTargetValue(
        juce::Decibels::decibelsToGain(
            pCompMakeup->load()));
    satMixSmoothed    .setTargetValue(
        pSatMix->load() / 100.0f);
    compMixSmoothed   .setTargetValue(
        pCompMix->load() / 100.0f);

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
    // MEASURE INPUT
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
    // STAGE 2 - SATURATION
    // skip() advances smoother properly across block
    // Returns end-of-block value for processing
    //──────────────────────────────────────────────
    if (!satBypassed)
    {
        float currentDrive  = driveSmoothed .skip(
            numSamples);
        float currentSatMix = satMixSmoothed.skip(
            numSamples);

        saturation.process(buffer,
                           currentDrive,
                           currentSatMix,
                           satModel);
    }
    else
    {
        // Advance smoothers when bypassed
        // Prevents stale state on re-enable
        driveSmoothed .skip(numSamples);
        satMixSmoothed.skip(numSamples);
    }

    //──────────────────────────────────────────────
    // SETUP COMPRESSOR
    // All parameters including extra model controls
    //──────────────────────────────────────────────
    compressor.setModel        (compModel);
    compressor.setThreshold    (compThresh);
    compressor.setRatio        (compRatio);
    compressor.setAttack       (compAttack);
    compressor.setRelease      (compRelease);
    compressor.setKnee         (compKnee);
    compressor.setFairchildTC  (fairchildTC);
    compressor.setAllButtonsIn (allButtonsIn);
    compressor.setThrustOn     (thrustOn);
    compressor.setFeedbackMode (feedbackMode);
    compressor.setLa2aLimit    (la2aLimit);
    compressor.setMakeupSmoothed(makeupSmoothed);
    compressor.setMixSmoothed  (compMixSmoothed);

    //──────────────────────────────────────────────
    // STAGES 3 AND 4 - EQ AND COMPRESSOR
    // Order depends on eqPreComp switch
    // Advance smoothers when bypassed
    //──────────────────────────────────────────────
    if (eqPreComp)
    {
        if (!eqBypassed)
            equalizer.process(buffer);

        if (!compBypassed)
        {
            compressor.process(buffer);
        }
        else
        {
            makeupSmoothed .skip(numSamples);
            compMixSmoothed.skip(numSamples);
        }
    }
    else
    {
        if (!compBypassed)
        {
            compressor.process(buffer);
        }
        else
        {
            makeupSmoothed .skip(numSamples);
            compMixSmoothed.skip(numSamples);
        }

        if (!eqBypassed)
            equalizer.process(buffer);
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
    // OUTPUT SOFT CLIPPER
    //──────────────────────────────────────────────
    for (int ch = 0; ch < numChannels; ch++)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; i++)
            data[i] = AnalogMath::softClip(data[i]);
    }

    //──────────────────────────────────────────────
    // MEASURE OUTPUT
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

    //──────────────────────────────────────────────
    // INPUT / OUTPUT
    //──────────────────────────────────────────────
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "inputGain",  "Input Gain",
            -24.0f, 24.0f, 0.0f));
    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "outputGain", "Output Gain",
            -24.0f, 24.0f, 0.0f));

    //──────────────────────────────────────────────
    // SATURATION
    //──────────────────────────────────────────────
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
                "NEVE","SSL","API",
                "TUBE","TAPE","FET","IRON"},
            0));
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "satBypass", "Sat Bypass", false));

    //──────────────────────────────────────────────
    // COMPRESSOR
    //──────────────────────────────────────────────
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

    //──────────────────────────────────────────────
    // COMPRESSOR EXTRA CONTROLS
    //──────────────────────────────────────────────

    // Fairchild 6-position time constant selector
    params.push_back(
        std::make_unique<juce::AudioParameterChoice>(
            "fairchildTC", "Fairchild TC",
            juce::StringArray{
                "TC 1  (0.2ms / 0.3s)",
                "TC 2  (0.2ms / 0.8s)",
                "TC 3  (0.4ms / 2.0s)",
                "TC 4  (0.4ms / Auto)",
                "TC 5  (0.4ms / 5.0s)",
                "TC 6  (0.4ms / Auto fast)"},
            0));

    // 1176 all-buttons-in mode
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "allButtonsIn", "All Buttons In", false));

    // API 2500 Thrust on/off
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "thrustOn", "Thrust On", true));

    // API 2500 topology FWD/BACK
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "feedbackMode", "Feedback Mode", false));

    // LA-2A compress/limit mode
    params.push_back(
        std::make_unique<juce::AudioParameterBool>(
            "la2aLimit", "LA2A Limit", false));

    //──────────────────────────────────────────────
    // EQ
    //──────────────────────────────────────────────
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