#pragma once
#include <JuceHeader.h>
#include "AnalogMath.h"
#include "SaturationProcessor.h"
#include "CompressorProcessor.h"
#include "EQProcessor.h"
#include "PresetManager.h"

class ModulatedStripProcessor : public juce::AudioProcessor
{
public:
    ModulatedStripProcessor();
    ~ModulatedStripProcessor() override;

    void prepareToPlay(double sampleRate,
                       int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&,
                      juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override
        { return "Modulated Strip"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override
        { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override
        { return {}; }
    void changeProgramName(int, const juce::String&)
        override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // Preset manager - public so editor can access
    PresetManager presetManager;

    float getOutputPeak()    const
        { return outputPeak.load(); }
    float getInputPeak()     const
        { return inputPeak.load(); }
    float getGainReduction() const
        { return compressor.getGainReduction(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout
        createParameters();

    SaturationProcessor saturation;
    CompressorProcessor compressor;
    EQProcessor         equalizer;

    // Cached parameter pointers
    std::atomic<float>* pInputGain     = nullptr;
    std::atomic<float>* pOutputGain    = nullptr;
    std::atomic<float>* pDrive         = nullptr;
    std::atomic<float>* pSatMix        = nullptr;
    std::atomic<float>* pSatModel      = nullptr;
    std::atomic<float>* pSatBypass     = nullptr;
    std::atomic<float>* pCompModel     = nullptr;
    std::atomic<float>* pCompThreshold = nullptr;
    std::atomic<float>* pCompRatio     = nullptr;
    std::atomic<float>* pCompAttack    = nullptr;
    std::atomic<float>* pCompRelease   = nullptr;
    std::atomic<float>* pCompMakeup    = nullptr;
    std::atomic<float>* pCompMix       = nullptr;
    std::atomic<float>* pCompKnee      = nullptr;
    std::atomic<float>* pCompBypass    = nullptr;
    std::atomic<float>* pFairchildTC   = nullptr;
    std::atomic<float>* pAllButtonsIn  = nullptr;
    std::atomic<float>* pThrustOn      = nullptr;
    std::atomic<float>* pFeedbackMode  = nullptr;
    std::atomic<float>* pLa2aLimit     = nullptr;
    std::atomic<float>* pEqModel       = nullptr;
    std::atomic<float>* pEqLowGain     = nullptr;
    std::atomic<float>* pEqLowFreq     = nullptr;
    std::atomic<float>* pEqMidGain     = nullptr;
    std::atomic<float>* pEqMidFreq     = nullptr;
    std::atomic<float>* pEqMidQ        = nullptr;
    std::atomic<float>* pEqHighGain    = nullptr;
    std::atomic<float>* pEqHighFreq    = nullptr;
    std::atomic<float>* pEqHPF         = nullptr;
    std::atomic<float>* pEqBypass      = nullptr;
    std::atomic<float>* pEqPreComp     = nullptr;

    // Smoothed parameters
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> inputGainSmoothed;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> outputGainSmoothed;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> makeupSmoothed;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> satMixSmoothed;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear> compMixSmoothed;

    std::atomic<float> inputPeak  { 0.0f };
    std::atomic<float> outputPeak { 0.0f };

    inline float safeTanh(float x)
    {
        if (x >  3.0f) return  1.0f;
        if (x < -3.0f) return -1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    inline float softClip(float x)
    {
        const float ceiling = 0.944f;
        if (x > ceiling || x < -ceiling)
            return ceiling * safeTanh(x / ceiling);
        return x;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        ModulatedStripProcessor)
};