#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>
#include "AnalogMath.h"

class CompressorProcessor
{
public:

    enum class Model
    {
        SSL_BUS   = 0,
        FAIRCHILD = 1,
        LA2A      = 2,
        FET_1176  = 3,
        API_2500  = 4
    };

    struct FairchildTC
    {
        float attackMs;
        float releaseMs;
        bool  programDependent;
    };

    static constexpr FairchildTC fairchildPositions[6] =
    {
        { 0.2f,   300.0f, false },
        { 0.2f,   800.0f, false },
        { 0.4f,  2000.0f, false },
        { 0.4f,  5000.0f, true  },
        { 0.4f,  5000.0f, false },
        { 0.4f, 10000.0f, true  }
    };

    void prepare(double sampleRate)
    {
        sr = sampleRate;
        float srf = static_cast<float>(sr);

        resetState();

        // Pre-compute fixed LA-2A coefficients
        la2aFastDecayCoeff   = AnalogMath::msToCoeff(
            40.0f, srf);
        la2aFixedAttackCoeff = AnalogMath::msToCoeff(
            10.0f, srf);

        // LA-2A charge coefficient bounds for lerp
        la2aChargeCoeffFast = AnalogMath::msToCoeff(
            10.0f, srf);
        la2aChargeCoeffSlow = AnalogMath::msToCoeff(
            20.0f, srf);

        calcSSLSidechainHPF();
        calcThrustHPF(150.0f);
    }

    void setModel(int m)
    {
        Model newModel = static_cast<Model>(m);
        if (newModel != model)
        {
            model = newModel;
            resetState();
        }
    }

    void setThreshold  (float t) { threshold = t; }
    void setRatio      (float r) { ratio     = r; }
    void setAttack     (float a) { attackMs  = a; }
    void setRelease    (float r) { releaseMs = r; }
    void setKnee       (float k) { kneeDb    = k; }
    void setFairchildTC(int tc)
    {
        fairchildPosition = juce::jlimit(0, 5, tc);
    }

    // Extra model controls
    void setAllButtonsIn (bool b) { allButtonsIn  = b; }
    void setThrustOn     (bool b) { thrustOn      = b; }
    void setFeedbackMode (bool b) { feedbackMode  = b; }
    void setLa2aLimit    (bool b) { la2aLimit     = b; }

    // Smoothed parameters passed by reference
    void setMakeupSmoothed(
        juce::SmoothedValue<float,
            juce::ValueSmoothingTypes::Linear>& s)
    {
        makeupSmootherPtr = &s;
    }

    void setMixSmoothed(
        juce::SmoothedValue<float,
            juce::ValueSmoothingTypes::Linear>& s)
    {
        mixSmootherPtr = &s;
    }

    float getGainReduction() const
    {
        return currentGainReductionDb.load();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();
        float srf       = static_cast<float>(sr);

        // Time constants once per block
        float attackCoeff  = AnalogMath::msToCoeff(
            attackMs, srf);
        float releaseCoeff = AnalogMath::msToCoeff(
            releaseMs, srf);

        // LA-2A slow decay once per block
        float slowMs = std::min(
            500.0f + compressionDuration * 2000.0f,
            5000.0f);
        float la2aSlowDecayCoeff = AnalogMath::msToCoeff(
            slowMs, srf);

        for (int i = 0; i < numSamples; i++)
        {
            // Smoothed makeup and mix per sample
            float makeup = (makeupSmootherPtr != nullptr)
                ? makeupSmootherPtr->getNextValue()
                : 1.0f;
            float mix = (mixSmootherPtr != nullptr)
                ? mixSmootherPtr->getNextValue()
                : 1.0f;

            float gainReductionDb = 0.0f;

            switch (model)
            {
                case Model::SSL_BUS:
                {
                    // RMS detection summed L+R
                    float sumSq = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                    {
                        float s = buffer.getSample(ch, i);
                        s = applySSLSidechainHPF(s, ch);
                        sumSq += s * s;
                    }
                    float rms = std::sqrt(sumSq
                        / static_cast<float>(
                            std::max(numChannels, 1)));

                    float W = 0.001f * srf;
                    float rmsCoeff = juce::jlimit(
                        0.0f, 0.999f, (W - 1.0f) / W);
                    sslRmsState = rmsCoeff * sslRmsState
                                + (1.0f - rmsCoeff) * rms;

                    float sidechainDb =
                        juce::Decibels::gainToDecibels(
                            sslRmsState, -96.0f);

                    gainReductionDb = calcSSL(
                        sidechainDb,
                        attackCoeff,
                        releaseCoeff);
                    break;
                }

                case Model::FAIRCHILD:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(
                            buffer.getSample(ch, i)));
                    float scDb =
                        juce::Decibels::gainToDecibels(
                            sc, -96.0f);
                    gainReductionDb = calcFairchild(scDb);
                    break;
                }

                case Model::LA2A:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(
                            buffer.getSample(ch, i)));
                    float scDb =
                        juce::Decibels::gainToDecibels(
                            sc, -96.0f);
                    gainReductionDb = calcLA2A(
                        scDb, sc, la2aSlowDecayCoeff);
                    break;
                }

                case Model::FET_1176:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(
                            buffer.getSample(ch, i)));
                    float scDb =
                        juce::Decibels::gainToDecibels(
                            sc, -96.0f);
                    gainReductionDb = calcFET1176(scDb);
                    break;
                }

                case Model::API_2500:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(
                            buffer.getSample(ch, i)));

                    // Feedback mode uses previous output
                    if (feedbackMode)
                    {
                        float fbSc = 0.0f;
                        for (int ch = 0; ch < numChannels; ch++)
                            fbSc = std::max(fbSc,
                                std::abs(prevOutput[ch]));
                        sc = fbSc;
                    }

                    gainReductionDb = calcAPI2500(
                        sc, releaseCoeff);
                    break;
                }
            }

            currentGainReductionDb.store(gainReductionDb);

            float grLinear =
                juce::Decibels::decibelsToGain(
                    gainReductionDb);

            for (int ch = 0; ch < numChannels; ch++)
            {
                float dry = buffer.getSample(ch, i);
                float wet = dry * grLinear * makeup;
                wet = applyModelColor(wet);

                float output = dry * (1.0f - mix)
                             + wet * mix;

                buffer.setSample(ch, i, output);
            }

            // Store output for API feedback topology
            if (model == Model::API_2500 && feedbackMode)
            {
                for (int ch = 0; ch < numChannels; ch++)
                    prevOutput[ch] =
                        buffer.getSample(ch, i);
            }
        }
    }

private:
    double sr        = 44100.0;
    Model  model     = Model::SSL_BUS;
    float  threshold = -20.0f;
    float  ratio     = 4.0f;
    float  attackMs  = 10.0f;
    float  releaseMs = 100.0f;
    float  kneeDb    = 6.0f;
    int    fairchildPosition = 0;

    // Extra model controls
    bool allButtonsIn  = false;
    bool thrustOn      = true;
    bool feedbackMode  = false;
    bool la2aLimit     = false;

    // Single stereo-linked envelope
    float envelope = 0.0f;

    // LA-2A dual opto state
    float optoFast = 0.0f;
    float optoSlow = 0.0f;

    // SSL state
    float sslRmsState         = 0.0f;
    float compressionActiveMs = 0.0f;

    // LA-2A photon memory
    float compressionDuration = 0.0f;

    // Pre-computed fixed LA-2A coefficients
    float la2aFastDecayCoeff   = 0.0f;
    float la2aFixedAttackCoeff = 0.0f;
    float la2aChargeCoeffFast  = 0.0f;
    float la2aChargeCoeffSlow  = 0.0f;

    // API feedback mode state
    float prevOutput[2] = { 0.0f, 0.0f };

    // SSL 30Hz sidechain HPF state (per channel)
    double sslHpfB0 = 1.0, sslHpfB1 = 0.0;
    double sslHpfA1 = 0.0;
    double sslHpfX1[2] = { 0.0, 0.0 };
    double sslHpfY1[2] = { 0.0, 0.0 };

    // API Thrust HPF state (mono by design)
    double thrustB0 = 1.0, thrustB1 = 0.0;
    double thrustB2 = 0.0, thrustA1 = 0.0;
    double thrustA2 = 0.0;
    double thrustX1 = 0.0, thrustX2 = 0.0;
    double thrustY1 = 0.0, thrustY2 = 0.0;

    // Smoothed parameter pointers
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>*
        makeupSmootherPtr = nullptr;

    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>*
        mixSmootherPtr = nullptr;

    std::atomic<float> currentGainReductionDb{ 0.0f };

    //──────────────────────────────────────────────
    // RESET STATE
    //──────────────────────────────────────────────
    void resetState()
    {
        envelope            = 0.0f;
        optoFast            = 0.0f;
        optoSlow            = 0.0f;
        sslRmsState         = 0.0f;
        compressionActiveMs = 0.0f;
        compressionDuration = 0.0f;
        prevOutput[0]       = 0.0f;
        prevOutput[1]       = 0.0f;

        for (int ch = 0; ch < 2; ch++)
        {
            sslHpfX1[ch] = 0.0;
            sslHpfY1[ch] = 0.0;
        }
        thrustX1 = thrustX2 = 0.0;
        thrustY1 = thrustY2 = 0.0;
    }

    //──────────────────────────────────────────────
    // GAIN REDUCTION COMPUTATION
    //──────────────────────────────────────────────
    float computeGainReduction(float inputDb)
    {
        float halfKnee  = kneeDb * 0.5f;
        float overshoot = inputDb - threshold;

        if (overshoot < -halfKnee)
            return 0.0f;

        if (overshoot < halfKnee && kneeDb > 0.0f)
        {
            float x = (overshoot + halfKnee) / kneeDb;
            overshoot = x * x * halfKnee;
        }

        return -(overshoot * (1.0f - 1.0f / ratio));
    }

    //──────────────────────────────────────────────
    // SSL 30Hz SIDECHAIN HPF
    //──────────────────────────────────────────────
    void calcSSLSidechainHPF()
    {
        double wc = 2.0
                  * juce::MathConstants<double>::pi
                  * 30.0 / sr;
        double k  = std::tan(wc / 2.0);
        sslHpfB0  =  1.0 / (1.0 + k);
        sslHpfB1  = -sslHpfB0;
        sslHpfA1  = (k - 1.0) / (k + 1.0);
    }

    float applySSLSidechainHPF(float x, int ch)
    {
        double y = sslHpfB0 * x
                 + sslHpfB1 * sslHpfX1[ch]
                 - sslHpfA1 * sslHpfY1[ch];
        sslHpfX1[ch] = x;
        sslHpfY1[ch] = y;
        return static_cast<float>(y);
    }

    //──────────────────────────────────────────────
    // SSL BUS COMPRESSOR
    // RMS detection, auto-release state machine
    //──────────────────────────────────────────────
    float calcSSL(float sidechainDb,
                  float attackCoeff,
                  float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);

        // Auto-release state machine
        // Two capacitors switch at 160ms threshold
        if (targetGr < 0.0f)
            compressionActiveMs +=
                1000.0f / static_cast<float>(sr);
        else
            compressionActiveMs = 0.0f;

        float releaseToUse = (compressionActiveMs > 160.0f)
            ? std::min(releaseMs, 40.0f)
            : releaseMs;

        float adaptiveRelease = AnalogMath::msToCoeff(
            releaseToUse, static_cast<float>(sr));

        if (targetGr < envelope)
            envelope = attackCoeff  * envelope
                     + (1.0f - attackCoeff)  * targetGr;
        else
            envelope = adaptiveRelease * envelope
                     + (1.0f - adaptiveRelease) * targetGr;

        return envelope;
    }

    //──────────────────────────────────────────────
    // FAIRCHILD 670
    // 6-position time constants from TC selector
    //──────────────────────────────────────────────
    float calcFairchild(float sidechainDb)
    {
        float srf = static_cast<float>(sr);

        const FairchildTC& tc =
            fairchildPositions[fairchildPosition];

        float aCoeff = AnalogMath::msToCoeff(
            tc.attackMs, srf);

        float releaseToUse = tc.releaseMs;
        if (tc.programDependent)
        {
            float grDepth = std::abs(envelope);
            releaseToUse  = std::min(
                tc.releaseMs + grDepth * 1000.0f,
                10000.0f);
        }

        float rCoeff   = AnalogMath::msToCoeff(
            releaseToUse, srf);
        float targetGr = computeGainReduction(sidechainDb);

        if (targetGr < envelope)
            envelope = aCoeff * envelope
                     + (1.0f - aCoeff) * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    //──────────────────────────────────────────────
    // LA-2A OPTICAL
    // Dual opto cell, fixed attack, photon memory
    // la2aLimit controls compress (3:1) vs limit (10:1)
    //──────────────────────────────────────────────
    float calcLA2A(float sidechainDb,
                   float sidechain,
                   float slowDecayCoeff)
    {
        // Temporarily set ratio based on mode
        float savedRatio = ratio;
        ratio = la2aLimit ? 10.0f : 3.0f;

        // Lerp charge coefficient from pre-computed bounds
        // Eliminates per-sample exp() call
        float norm = juce::jlimit(0.0f, 1.0f,
            (sidechainDb + 60.0f) / 60.0f);
        float chargeCoeff = la2aChargeCoeffSlow
            + norm * (la2aChargeCoeffFast
                    - la2aChargeCoeffSlow);

        // Dual opto cell
        if (sidechain > optoFast)
            optoFast = chargeCoeff * optoFast
                     + (1.0f - chargeCoeff) * sidechain;
        else
            optoFast = la2aFastDecayCoeff * optoFast
                     + (1.0f - la2aFastDecayCoeff)
                     * sidechain;

        if (sidechain > optoSlow)
            optoSlow = chargeCoeff * optoSlow
                     + (1.0f - chargeCoeff) * sidechain;
        else
            optoSlow = slowDecayCoeff * optoSlow
                     + (1.0f - slowDecayCoeff) * sidechain;

        float optoState = 0.6f * optoFast
                        + 0.4f * optoSlow;

        // CdS photon memory
        if (std::abs(envelope) > 1.0f)
            compressionDuration +=
                1.0f / static_cast<float>(sr);
        else
            compressionDuration *= 0.999f;
        compressionDuration = std::min(
            compressionDuration, 10.0f);

        float optoDb =
            juce::Decibels::gainToDecibels(
                optoState, -96.0f);
        float targetGr = computeGainReduction(optoDb);

        // Release stage selection
        float rCoeff = (std::abs(envelope) > 6.0f)
            ? slowDecayCoeff
            : la2aFastDecayCoeff;

        // Fixed attack - no user control on LA-2A
        if (targetGr < envelope)
            envelope = la2aFixedAttackCoeff * envelope
                     + (1.0f - la2aFixedAttackCoeff)
                     * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        // Restore original ratio
        ratio = savedRatio;

        return envelope;
    }

    //──────────────────────────────────────────────
    // 1176 FET
    // Clean signature - computes own timing
    // Supports all-buttons-in mode
    //──────────────────────────────────────────────
    float calcFET1176(float sidechainDb)
    {
        float srf = static_cast<float>(sr);

        float targetGr;

        if (allButtonsIn)
        {
            // All-buttons-in mode
            // Ratio evolves with GR depth
            // Gets harder as compression increases
            float grDepth    = std::abs(envelope);
            float allInRatio = 12.0f + grDepth * 0.8f;
            float savedRatio = ratio;
            float savedKnee  = kneeDb;

            ratio  = allInRatio;
            kneeDb = 12.0f;
            targetGr = computeGainReduction(sidechainDb);

            ratio  = savedRatio;
            kneeDb = savedKnee;
        }
        else
        {
            targetGr = computeGainReduction(sidechainDb);
        }

        // Map user attack to authentic 1176 range
        // 0.02ms to 0.8ms logarithmically
        float normAttack =
            (attackMs - 0.1f) / 99.9f;
        float mappedAttack = 0.02f
            * std::pow(40.0f, normAttack);
        float fetAttack = AnalogMath::msToCoeff(
            mappedAttack, srf);

        // Map user release to authentic 1176 range
        // 50ms to 1100ms logarithmically
        float normRelease =
            (releaseMs - 10.0f) / 1990.0f;
        float mappedRelease = 50.0f
            * std::pow(22.0f, normRelease);
        float fetRelease = AnalogMath::msToCoeff(
            mappedRelease, srf);

        if (targetGr < envelope)
            envelope = fetAttack  * envelope
                     + (1.0f - fetAttack)  * targetGr;
        else
            envelope = fetRelease * envelope
                     + (1.0f - fetRelease) * targetGr;

        return envelope;
    }

    //──────────────────────────────────────────────
    // API 2500
    // Clean signature - no dead parameters
    // Thrust toggleable, feedback mode supported
    //──────────────────────────────────────────────
    float calcAPI2500(float rawSidechain,
                      float releaseCoeff)
    {
        float srf = static_cast<float>(sr);

        // Apply Thrust HPF only when enabled
        float sidechain = thrustOn
            ? applyThrustHPF(rawSidechain)
            : rawSidechain;

        float sidechainDb =
            juce::Decibels::gainToDecibels(
                std::abs(sidechain), -96.0f);

        float targetGr = computeGainReduction(sidechainDb);

        float apiAttack = AnalogMath::msToCoeff(
            attackMs * 0.8f, srf);

        if (targetGr < envelope)
            envelope = apiAttack  * envelope
                     + (1.0f - apiAttack)  * targetGr;
        else
            envelope = releaseCoeff * envelope
                     + (1.0f - releaseCoeff) * targetGr;

        return envelope;
    }

    //──────────────────────────────────────────────
    // API THRUST HPF
    // 2nd order Butterworth at 150Hz
    //──────────────────────────────────────────────
    void calcThrustHPF(float freq)
    {
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * 0.707);

        double b0 = (1.0 + cosw0) / 2.0;
        double b1 = -(1.0 + cosw0);
        double b2 = (1.0 + cosw0) / 2.0;
        double a0 =  1.0 + alpha;
        double a1 = -2.0 * cosw0;
        double a2 =  1.0 - alpha;

        thrustB0 = b0 / a0; thrustB1 = b1 / a0;
        thrustB2 = b2 / a0; thrustA1 = a1 / a0;
        thrustA2 = a2 / a0;
    }

    float applyThrustHPF(float x)
    {
        double y = thrustB0 * x
                 + thrustB1 * thrustX1
                 + thrustB2 * thrustX2
                 - thrustA1 * thrustY1
                 - thrustA2 * thrustY2;
        thrustX2 = thrustX1; thrustX1 = x;
        thrustY2 = thrustY1; thrustY1 = y;
        return static_cast<float>(y);
    }

    //──────────────────────────────────────────────
    // MODEL COLORATION
    //──────────────────────────────────────────────
    float applyModelColor(float x)
    {
        switch (model)
        {
            case Model::FAIRCHILD:
                return AnalogMath::safeTanh(x * 1.1f)
                     / 1.08f;

            case Model::LA2A:
                return x + 0.015f * x * x
                     * (x > 0.0f ? 1.0f : -1.0f);

            case Model::FET_1176:
            {
                float colored = x + 0.02f * x * x * x;
                // All-buttons-in adds extra saturation
                if (allButtonsIn)
                    colored += 0.06f * x * x * x;
                return colored;
            }

            case Model::SSL_BUS:
            case Model::API_2500:
            default:
                return x;
        }
    }
};