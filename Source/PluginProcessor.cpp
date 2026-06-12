#include "PluginProcessor.h"
#include "PluginEditor.h"

ModulatedStripProcessor::ModulatedStripProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput ("Input",
                juce::AudioChannelSet::stereo(), true)
            .withOutput("Output",
                juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters",
            createParameters()),
      presetManager(apvts)
{
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
    pFairchildTC   = apvts.getRawParameterValue("fairchildTC");
    pAllButtonsIn  = apvts.getRawParameterValue("allButtonsIn");
    pThrustOn      = apvts.getRawParameterValue("thrustOn");
    pFeedbackMode  = apvts.getRawParameterValue("feedbackMode");
    pLa2aLimit     = apvts.getRawParameterValue("la2aLimit");
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
    pOversample    = apvts.getRawParameterValue("oversample");
    pDelta         = apvts.getRawParameterValue("delta");
    pAnalogBypass  = apvts.getRawParameterValue("analogBypass");
    pStereoMode    = apvts.getRawParameterValue("stereoMode");
    pCrosstalk     = apvts.getRawParameterValue("crosstalk");
    pNoiseFloor    = apvts.getRawParameterValue("noiseFloor");
}

ModulatedStripProcessor::~ModulatedStripProcessor() {}

void ModulatedStripProcessor::setupOversampling(
    int factor, int maxBlockSize)
{
    if (factor == currentOversampleFactor
     && oversampling != nullptr)
        return;

    currentOversampleFactor = factor;

    int order = 0;
    if      (factor == 2) order = 1;
    else if (factor == 4) order = 2;
    else if (factor == 8) order = 3;

    if (order > 0)
    {
        oversampling = std::make_unique<
            juce::dsp::Oversampling<float>>(
            2, order,
            juce::dsp::Oversampling<float>
                ::filterHalfBandPolyphaseIIR,
            true);
        oversampling->initProcessing(
            static_cast<size_t>(maxBlockSize));
    }
    else
    {
        oversampling.reset();
    }
}

void ModulatedStripProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    saturation.prepare(sampleRate);
    compressor.prepare(sampleRate);
    equalizer .prepare(sampleRate);
    analogBypassSat.prepare(sampleRate);

    double sr = sampleRate;

    inputGainSmoothed .reset(sr, 0.010);
    outputGainSmoothed.reset(sr, 0.010);
    driveSmoothed     .reset(sr, 0.020);
    makeupSmoothed    .reset(sr, 0.010);
    satMixSmoothed    .reset(sr, 0.020);
    compMixSmoothed   .reset(sr, 0.020);

    inputGainSmoothed .setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(pInputGain->load()));
    outputGainSmoothed.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(pOutputGain->load()));
    driveSmoothed     .setCurrentAndTargetValue(
        pDrive->load() / 100.0f);
    makeupSmoothed    .setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(pCompMakeup->load()));
    satMixSmoothed    .setCurrentAndTargetValue(
        pSatMix->load() / 100.0f);
    compMixSmoothed   .setCurrentAndTargetValue(
        pCompMix->load() / 100.0f);

    // P1 FIX - dry buffer allocated here not in processBlock
    dryBuffer.setSize(2, samplesPerBlock,
        false, true, false);

    // FIX - pendingOsFactor applied at prepareToPlay
    // This is where oversampling changes actually take effect
    // pendingOsFactor is set in processBlock when user changes
    // the selector, then host calls prepareToPlay on latency change
    int osFactor = (pendingOsFactor != currentOversampleFactor)
        ? pendingOsFactor
        : static_cast<int>(pOversample->load());

    setupOversampling(osFactor, samplesPerBlock);
    pendingOsFactor = osFactor;

    // Reset LUFS smoother
    lufsSmoothed = 0.0f;
}

void ModulatedStripProcessor::releaseResources()
{
    if (oversampling != nullptr)
        oversampling->reset();
}

juce::AudioProcessorEditor*
ModulatedStripProcessor::createEditor()
{
    return new ModulatedStripEditor(*this);
}

void ModulatedStripProcessor::encodeMS(
    juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float L = buffer.getSample(0, i);
        float R = buffer.getSample(1, i);
        buffer.setSample(0, i, (L + R) * 0.5f);
        buffer.setSample(1, i, (L - R) * 0.5f);
    }
}

void ModulatedStripProcessor::decodeMS(
    juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float M = buffer.getSample(0, i);
        float S = buffer.getSample(1, i);
        buffer.setSample(0, i, M + S);
        buffer.setSample(1, i, M - S);
    }
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
    float inputGainDb  = pInputGain->load();
    float outputGainDb = pOutputGain->load();
    int   satModel     = static_cast<int>(pSatModel->load());
    bool  satBypassed  = pSatBypass->load() > 0.5f;

    int   compModel    = static_cast<int>(pCompModel->load());
    float compThresh   = pCompThreshold->load();
    float compRatio    = pCompRatio->load();
    float compAttack   = pCompAttack->load();
    float compRelease  = pCompRelease->load();
    float compKnee     = pCompKnee->load();
    bool  compBypassed = pCompBypass->load() > 0.5f;
    int   fairchildTC  = static_cast<int>(pFairchildTC->load());
    bool  allButtonsIn = pAllButtonsIn->load() > 0.5f;
    bool  thrustOn     = pThrustOn->load()     > 0.5f;
    bool  feedbackMode = pFeedbackMode->load() > 0.5f;
    bool  la2aLimit    = pLa2aLimit->load()    > 0.5f;

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
    bool  eqPreComp    = pEqPreComp->load() > 0.5f;

    int   osFactor     = static_cast<int>(pOversample->load());
    bool  deltaMode    = pDelta->load()        > 0.5f;
    bool  analogBypass = pAnalogBypass->load() > 0.5f;
    int   stereoMode   = static_cast<int>(pStereoMode->load());
    bool  crosstalkOn  = pCrosstalk->load()    > 0.5f;
    bool  noiseFloorOn = pNoiseFloor->load()   > 0.5f;

    //──────────────────────────────────────────────
    // FIX - oversampling change triggers host refresh
    // updateHostDisplay() signals latency change
    // host calls prepareToPlay where change is applied
    //──────────────────────────────────────────────
    if (osFactor != currentOversampleFactor
     && osFactor != pendingOsFactor)
    {
        pendingOsFactor = osFactor;
        // Notify host of latency change
        // This triggers prepareToPlay on most hosts
        // where the actual oversampling rebuild happens
        updateHostDisplay();
    }

    //──────────────────────────────────────────────
    // SMOOTHED TARGETS
    //──────────────────────────────────────────────
    inputGainSmoothed .setTargetValue(
        juce::Decibels::decibelsToGain(inputGainDb));
    outputGainSmoothed.setTargetValue(
        juce::Decibels::decibelsToGain(outputGainDb));
    driveSmoothed     .setTargetValue(pDrive->load() / 100.0f);
    makeupSmoothed    .setTargetValue(
        juce::Decibels::decibelsToGain(pCompMakeup->load()));
    satMixSmoothed    .setTargetValue(pSatMix->load() / 100.0f);
    compMixSmoothed   .setTargetValue(pCompMix->load() / 100.0f);

    //──────────────────────────────────────────────
    // UPDATE EQ
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
    // DELTA MODE
    //──────────────────────────────────────────────
    if (deltaMode)
    {
        for (int ch = 0; ch < std::min(numChannels, 2); ch++)
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    //──────────────────────────────────────────────
    // ANALOG BYPASS
    //──────────────────────────────────────────────
    if (analogBypass)
    {
        analogBypassSat.process(buffer, 0.08f, 1.0f, 6);
        for (int i = 0; i < numSamples; i++)
        {
            float g = outputGainSmoothed.getNextValue();
            for (int ch = 0; ch < numChannels; ch++)
                buffer.setSample(ch, i,
                    buffer.getSample(ch, i) * g);
        }
        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; i++)
                data[i] = AnalogMath::softClip(data[i]);
        }
        inputGainSmoothed.skip(numSamples);
        driveSmoothed    .skip(numSamples);
        satMixSmoothed   .skip(numSamples);
        makeupSmoothed   .skip(numSamples);
        compMixSmoothed  .skip(numSamples);

        float outPeak = 0.0f;
        for (int ch = 0; ch < numChannels; ch++)
            outPeak = std::max(outPeak,
                buffer.getMagnitude(ch, 0, numSamples));
        outputPeak.store(outPeak);
        updateLUFS(buffer, numSamples);
        return;
    }

    //──────────────────────────────────────────────
    // STAGE 1 - INPUT GAIN
    //──────────────────────────────────────────────
    for (int i = 0; i < numSamples; i++)
    {
        float g = inputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }

    //──────────────────────────────────────────────
    // MID/SIDE ENCODE
    //──────────────────────────────────────────────
    if (stereoMode == 1) encodeMS(buffer);

    //──────────────────────────────────────────────
    // STAGE 2 - SATURATION
    //──────────────────────────────────────────────
    if (!satBypassed)
    {
        float currentDrive  = driveSmoothed .skip(numSamples);
        float currentSatMix = satMixSmoothed.skip(numSamples);

        if (oversampling != nullptr
         && currentOversampleFactor > 1)
        {
            juce::dsp::AudioBlock<float> block(buffer);
            auto osBlock = oversampling->processSamplesUp(block);
            int osN  = static_cast<int>(osBlock.getNumSamples());
            int osCh = static_cast<int>(osBlock.getNumChannels());
            juce::AudioBuffer<float> osBuffer(osCh, osN);
            for (int ch = 0; ch < osCh; ch++)
                osBuffer.copyFrom(ch, 0,
                    osBlock.getChannelPointer(ch), osN);
            saturation.process(osBuffer,
                currentDrive, currentSatMix, satModel);
            for (int ch = 0; ch < osCh; ch++)
                juce::FloatVectorOperations::copy(
                    osBlock.getChannelPointer(ch),
                    osBuffer.getReadPointer(ch), osN);
            oversampling->processSamplesDown(block);
        }
        else
        {
            saturation.process(buffer,
                currentDrive, currentSatMix, satModel);
        }
    }
    else
    {
        driveSmoothed .skip(numSamples);
        satMixSmoothed.skip(numSamples);
    }

    //──────────────────────────────────────────────
    // SETUP COMPRESSOR
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
    compressor.setCrosstalk    (crosstalkOn);
    compressor.setNoiseFloor   (noiseFloorOn);
    compressor.setMakeupSmoothed(makeupSmoothed);
    compressor.setMixSmoothed  (compMixSmoothed);

    //──────────────────────────────────────────────
    // STAGES 3 AND 4
    //──────────────────────────────────────────────
    if (eqPreComp)
    {
        if (!eqBypassed)   equalizer.process(buffer);
        if (!compBypassed) compressor.process(buffer);
        else
        {
            makeupSmoothed .skip(numSamples);
            compMixSmoothed.skip(numSamples);
        }
    }
    else
    {
        if (!compBypassed) compressor.process(buffer);
        else
        {
            makeupSmoothed .skip(numSamples);
            compMixSmoothed.skip(numSamples);
        }
        if (!eqBypassed) equalizer.process(buffer);
    }

    //──────────────────────────────────────────────
    // MID/SIDE DECODE
    //──────────────────────────────────────────────
    if (stereoMode == 1) decodeMS(buffer);

    //──────────────────────────────────────────────
    // STAGE 5 - OUTPUT GAIN
    //──────────────────────────────────────────────
    for (int i = 0; i < numSamples; i++)
    {
        float g = outputGainSmoothed.getNextValue();
        for (int ch = 0; ch < numChannels; ch++)
            buffer.setSample(ch, i,
                buffer.getSample(ch, i) * g);
    }

    //──────────────────────────────────────────────
    // DELTA MODE - subtract dry
    //──────────────────────────────────────────────
    if (deltaMode)
    {
        for (int ch = 0; ch < std::min(numChannels, 2); ch++)
        {
            float* wet = buffer.getWritePointer(ch);
            const float* dry = dryBuffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; i++)
                wet[i] = wet[i] - dry[i];
        }
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

    // FIX - LUFS computed from output buffer
    // Feeds the LUFSMeter component via atomics
    updateLUFS(buffer, numSamples);
}

void ModulatedStripProcessor::updateLUFS(
    const juce::AudioBuffer<float>& buffer,
    int numSamples)
{
    // Simplified K-weighted short-term LUFS
    // Full ITU-R BS.1770 requires pre-filter and gating
    // This is a display approximation updated per block
    int numCh = buffer.getNumChannels();
    float sumSq = 0.0f;

    for (int ch = 0; ch < numCh; ch++)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; i++)
            sumSq += data[i] * data[i];
    }

    float rms = std::sqrt(sumSq /
        static_cast<float>(
            std::max(numCh, 1) * numSamples));

    // 3-second integration (approximate)
    float coeff = std::exp(-1.0f /
        (static_cast<float>(getSampleRate())
         * 3.0f
         / static_cast<float>(numSamples)));

    lufsSmoothed = coeff * lufsSmoothed
                 + (1.0f - coeff) * rms;

    // Store for GUI thread to read
    // LUFSMeter reads this via getLUFS()
    currentLUFS.store(-0.691f + 10.0f
        * std::log10(lufsSmoothed * lufsSmoothed
            * static_cast<float>(numCh) + 1e-10f));
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
        apvts.replaceState(
            juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout
ModulatedStripProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>>
        params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "inputGain", "Input Gain", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain", "Output Gain", -24.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive", 0.0f, 100.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "satMix", "Sat Mix", 0.0f, 100.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "satModel", "Sat Model",
        juce::StringArray{"NEVE","SSL","API","TUBE","TAPE","FET","IRON"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "satBypass", "Sat Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "compModel", "Comp Model",
        juce::StringArray{"SSL Bus","Fairchild 670","LA-2A","1176","API 2500"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compThreshold", "Threshold", -60.0f, 0.0f, -20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRatio", "Ratio", 1.0f, 20.0f, 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compAttack", "Attack", 0.1f, 100.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compRelease", "Release", 10.0f, 2000.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compMakeup", "Makeup", 0.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compMix", "Comp Mix", 0.0f, 100.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compKnee", "Knee", 0.0f, 12.0f, 6.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "compBypass", "Comp Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "fairchildTC", "Fairchild TC",
        juce::StringArray{
            "TC 1 (0.2ms/0.3s)","TC 2 (0.2ms/0.8s)",
            "TC 3 (0.4ms/2.0s)","TC 4 (0.4ms/Auto)",
            "TC 5 (0.4ms/5.0s)","TC 6 (0.4ms/Auto fast)"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "allButtonsIn", "All Buttons In", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "thrustOn", "Thrust On", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "feedbackMode", "Feedback Mode", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "la2aLimit", "LA2A Limit", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "eqModel", "EQ Model",
        juce::StringArray{
            "Neve 1073","Neve 1084","SSL 4000E",
            "Pultec EQP-1A","API 550A"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqLowGain", "Low Gain", -15.0f, 15.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqLowFreq", "Low Freq", 30.0f, 300.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqMidGain", "Mid Gain", -15.0f, 15.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqMidFreq", "Mid Freq", 200.0f, 8000.0f, 1000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqMidQ", "Mid Q", 0.1f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqHighGain", "High Gain", -15.0f, 15.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqHighFreq", "High Freq", 1500.0f, 16000.0f, 10000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "eqHPF", "HPF", 20.0f, 500.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "eqBypass", "EQ Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "eqPreComp", "EQ Pre Comp", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "oversample", "Oversample",
        juce::StringArray{"1x (Off)","2x","4x","8x"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "delta", "Delta", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "analogBypass", "Analog Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "stereoMode", "Stereo Mode",
        juce::StringArray{"Stereo","Mid/Side","Dual Mono"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "crosstalk", "Crosstalk", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "noiseFloor", "Noise Floor", true));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModulatedStripProcessor();
}