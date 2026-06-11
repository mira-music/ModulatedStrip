#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>
#include <random>
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

        la2aFastDecayCoeff   = AnalogMath::msToCoeff(40.0f, srf);
        la2aFixedAttackCoeff = AnalogMath::msToCoeff(10.0f, srf);
        la2aChargeCoeffFast  = AnalogMath::msToCoeff(10.0f, srf);
        la2aChargeCoeffSlow  = AnalogMath::msToCoeff(20.0f, srf);

        calcSSLSidechainHPF();
        calcThrustHPF(150.0f);

        // Noise generator state
        noiseRng = std::mt19937(42);
        noiseDist = std::uniform_real_distribution<float>(
            -1.0f, 1.0f);

        // Noise pink filter state
        for (int ch = 0; ch < 2; ch++)
        {
            for (int b = 0; b < 7; b++)
                pinkState[ch][b] = 0.0f;
        }
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
    { fairchildPosition = juce::jlimit(0, 5, tc); }
    void setAllButtonsIn (bool b) { allButtonsIn  = b; }
    void setThrustOn     (bool b) { thrustOn      = b; }
    void setFeedbackMode (bool b) { feedbackMode  = b; }
    void setLa2aLimit    (bool b) { la2aLimit     = b; }
    void setCrosstalk    (bool b) { crosstalkOn   = b; }
    void setNoiseFloor   (bool b) { noiseFloorOn  = b; }

    void setMakeupSmoothed(
        juce::SmoothedValue<float,
            juce::ValueSmoothingTypes::Linear>& s)
    { makeupSmootherPtr = &s; }

    void setMixSmoothed(
        juce::SmoothedValue<float,
            juce::ValueSmoothingTypes::Linear>& s)
    { mixSmootherPtr = &s; }

    float getGainReduction() const
    { return currentGainReductionDb.load(); }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();
        float srf = static_cast<float>(sr);

        float attackCoeff  = AnalogMath::msToCoeff(attackMs, srf);
        float releaseCoeff = AnalogMath::msToCoeff(releaseMs, srf);

        float slowMs = std::min(
            500.0f + compressionDuration * 2000.0f,
            5000.0f);
        float la2aSlowDecayCoeff = AnalogMath::msToCoeff(slowMs, srf);

        // Model noise floor levels in dBFS
        float noiseLevel = 0.0f;
        if (noiseFloorOn)
        {
            switch (model)
            {
                case Model::SSL_BUS:   noiseLevel = 0.000003f; break; // -110dBFS
                case Model::FAIRCHILD: noiseLevel = 0.000010f; break; // -100dBFS
                case Model::LA2A:      noiseLevel = 0.000006f; break; // -104dBFS
                case Model::FET_1176:  noiseLevel = 0.000004f; break; // -108dBFS
                case Model::API_2500:  noiseLevel = 0.000003f; break; // -110dBFS
                default:               noiseLevel = 0.000003f; break;
            }
        }

        for (int i = 0; i < numSamples; i++)
        {
            float makeup = (makeupSmootherPtr != nullptr)
                ? makeupSmootherPtr->getNextValue() : 1.0f;
            float mix = (mixSmootherPtr != nullptr)
                ? mixSmootherPtr->getNextValue() : 1.0f;

            float gainReductionDb = 0.0f;

            switch (model)
            {
                case Model::SSL_BUS:
                {
                    float sumSq = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                    {
                        float s = buffer.getSample(ch, i);
                        s = applySSLSidechainHPF(s, ch);
                        sumSq += s * s;
                    }
                    float rms = std::sqrt(sumSq /
                        static_cast<float>(std::max(numChannels, 1)));
                    float W = 0.001f * srf;
                    float rmsCoeff = juce::jlimit(0.0f, 0.999f,
                        (W - 1.0f) / W);
                    sslRmsState = rmsCoeff * sslRmsState
                        + (1.0f - rmsCoeff) * rms;
                    float sidechainDb = juce::Decibels::gainToDecibels(
                        sslRmsState, -96.0f);
                    gainReductionDb = calcSSL(sidechainDb,
                        attackCoeff, releaseCoeff);
                    break;
                }
                case Model::FAIRCHILD:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(buffer.getSample(ch, i)));
                    float scDb = juce::Decibels::gainToDecibels(sc, -96.0f);
                    gainReductionDb = calcFairchild(scDb);
                    break;
                }
                case Model::LA2A:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(buffer.getSample(ch, i)));
                    float scDb = juce::Decibels::gainToDecibels(sc, -96.0f);
                    gainReductionDb = calcLA2A(scDb, sc, la2aSlowDecayCoeff);
                    break;
                }
                case Model::FET_1176:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(buffer.getSample(ch, i)));
                    float scDb = juce::Decibels::gainToDecibels(sc, -96.0f);
                    gainReductionDb = calcFET1176(scDb);
                    break;
                }
                case Model::API_2500:
                {
                    float sc = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                        sc = std::max(sc, std::abs(buffer.getSample(ch, i)));
                    if (feedbackMode)
                    {
                        float fbSc = 0.0f;
                        for (int ch = 0; ch < numChannels; ch++)
                            fbSc = std::max(fbSc, std::abs(prevOutput[ch]));
                        sc = fbSc;
                    }
                    gainReductionDb = calcAPI2500(sc, releaseCoeff);
                    break;
                }
            }

            currentGainReductionDb.store(gainReductionDb);
            float grLinear = juce::Decibels::decibelsToGain(gainReductionDb);

            // Apply compression
            float outL = 0.0f, outR = 0.0f;
            for (int ch = 0; ch < numChannels; ch++)
            {
                float dry = buffer.getSample(ch, i);
                float wet = dry * grLinear * makeup;
                wet = applyModelColor(wet);
                float output = dry * (1.0f - mix) + wet * mix;

                // Add model-specific noise floor
                if (noiseFloorOn && noiseLevel > 0.0f)
                {
                    float noise = generatePinkNoise(ch) * noiseLevel;
                    output += noise;
                }

                buffer.setSample(ch, i, output);

                if (ch == 0) outL = output;
                if (ch == 1) outR = output;
            }

            // P1 FIX - Inter-channel crosstalk
            // Two lines give the stereo "glue" of real hardware
            if (crosstalkOn && numChannels >= 2)
            {
                const float xtalk = 0.002f; // 0.2% = -54dBFS isolation
                float newL = outL + xtalk * outR;
                float newR = outR + xtalk * outL;
                buffer.setSample(0, i, newL);
                buffer.setSample(1, i, newR);
            }

            // Store for feedback topology
            if (model == Model::API_2500 && feedbackMode)
            {
                for (int ch = 0; ch < numChannels; ch++)
                    prevOutput[ch] = buffer.getSample(ch, i);
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

    bool allButtonsIn  = false;
    bool thrustOn      = true;
    bool feedbackMode  = false;
    bool la2aLimit     = false;
    bool crosstalkOn   = true;
    bool noiseFloorOn  = true;

    float envelope          = 0.0f;
    float optoFast          = 0.0f;
    float optoSlow          = 0.0f;
    float sslRmsState       = 0.0f;
    float compressionActiveMs = 0.0f;
    float compressionDuration = 0.0f;
    float prevOutput[2]     = { 0.0f, 0.0f };

    float la2aFastDecayCoeff   = 0.0f;
    float la2aFixedAttackCoeff = 0.0f;
    float la2aChargeCoeffFast  = 0.0f;
    float la2aChargeCoeffSlow  = 0.0f;

    // SSL HPF state
    double sslHpfB0 = 1.0, sslHpfB1 = 0.0, sslHpfA1 = 0.0;
    double sslHpfX1[2] = { 0.0, 0.0 };
    double sslHpfY1[2] = { 0.0, 0.0 };

    // P0 FIX - Thrust HPF per-channel state
    // Previously used single state for both channels - audio corruption bug
    double thrustB0 = 1.0, thrustB1 = 0.0;
    double thrustB2 = 0.0, thrustA1 = 0.0, thrustA2 = 0.0;
    double thrustX1[2] = { 0.0, 0.0 };
    double thrustX2[2] = { 0.0, 0.0 };
    double thrustY1[2] = { 0.0, 0.0 };
    double thrustY2[2] = { 0.0, 0.0 };

    // Noise floor
    std::mt19937 noiseRng;
    std::uniform_real_distribution<float> noiseDist;
    float pinkState[2][7] = {};

    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>* makeupSmootherPtr = nullptr;
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>* mixSmootherPtr = nullptr;

    std::atomic<float> currentGainReductionDb{ 0.0f };

    inline float safeTanh(float x)
    {
        if (x >  3.0f) return  1.0f;
        if (x < -3.0f) return -1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Pink noise generator (Paul Kellett approximation)
    float generatePinkNoise(int ch)
    {
        float white = noiseDist(noiseRng);
        auto& s = pinkState[ch];
        s[0] = 0.99886f * s[0] + white * 0.0555179f;
        s[1] = 0.99332f * s[1] + white * 0.0750759f;
        s[2] = 0.96900f * s[2] + white * 0.1538520f;
        s[3] = 0.86650f * s[3] + white * 0.3104856f;
        s[4] = 0.55000f * s[4] + white * 0.5329522f;
        s[5] = -0.7616f * s[5] - white * 0.0168980f;
        float pink = s[0] + s[1] + s[2] + s[3]
                   + s[4] + s[5] + s[6] + white * 0.5362f;
        s[6] = white * 0.115926f;
        return pink * 0.11f;
    }

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
            // P0 FIX - reset all per-channel thrust state
            thrustX1[ch] = thrustX2[ch] = 0.0;
            thrustY1[ch] = thrustY2[ch] = 0.0;
        }
    }

    float computeGainReduction(float inputDb)
    {
        float halfKnee  = kneeDb * 0.5f;
        float overshoot = inputDb - threshold;
        if (overshoot < -halfKnee) return 0.0f;
        if (overshoot < halfKnee && kneeDb > 0.0f)
        {
            float x = (overshoot + halfKnee) / kneeDb;
            overshoot = x * x * halfKnee;
        }
        return -(overshoot * (1.0f - 1.0f / ratio));
    }

    // P1 - accepts explicit ratio to avoid member mutation
    float computeGainReductionWith(
        float inputDb, float r, float knee)
    {
        float halfKnee  = knee * 0.5f;
        float overshoot = inputDb - threshold;
        if (overshoot < -halfKnee) return 0.0f;
        if (overshoot < halfKnee && knee > 0.0f)
        {
            float x = (overshoot + halfKnee) / knee;
            overshoot = x * x * halfKnee;
        }
        return -(overshoot * (1.0f - 1.0f / r));
    }

    void calcSSLSidechainHPF()
    {
        double wc = 2.0 * juce::MathConstants<double>::pi * 30.0 / sr;
        double k  = std::tan(wc / 2.0);
        sslHpfB0 =  1.0 / (1.0 + k);
        sslHpfB1 = -sslHpfB0;
        sslHpfA1 = (k - 1.0) / (k + 1.0);
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

    float calcSSL(float sidechainDb,
                  float attackCoeff,
                  float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);
        if (targetGr < 0.0f)
            compressionActiveMs += 1000.0f / static_cast<float>(sr);
        else
            compressionActiveMs = 0.0f;
        float releaseToUse = (compressionActiveMs > 160.0f)
            ? std::min(releaseMs, 40.0f) : releaseMs;
        float adaptiveRelease = AnalogMath::msToCoeff(
            releaseToUse, static_cast<float>(sr));
        if (targetGr < envelope)
            envelope = attackCoeff  * envelope + (1.0f - attackCoeff)  * targetGr;
        else
            envelope = adaptiveRelease * envelope + (1.0f - adaptiveRelease) * targetGr;
        return envelope;
    }

    float calcFairchild(float sidechainDb)
    {
        float srf = static_cast<float>(sr);
        const FairchildTC& tc = fairchildPositions[fairchildPosition];
        float aCoeff = AnalogMath::msToCoeff(tc.attackMs, srf);
        float releaseToUse = tc.releaseMs;
        if (tc.programDependent)
        {
            float grDepth = std::abs(envelope);
            releaseToUse = std::min(
                tc.releaseMs + grDepth * 1000.0f, 10000.0f);
        }
        float rCoeff   = AnalogMath::msToCoeff(releaseToUse, srf);
        float targetGr = computeGainReduction(sidechainDb);
        if (targetGr < envelope)
            envelope = aCoeff * envelope + (1.0f - aCoeff) * targetGr;
        else
            envelope = rCoeff * envelope + (1.0f - rCoeff) * targetGr;
        return envelope;
    }

    float calcLA2A(float sidechainDb,
                   float sidechain,
                   float slowDecayCoeff)
    {
        // P0 FIX - ratio no longer mutated as member variable
        // Use local variable passed to computeGainReductionWith()
        float la2aRatio = la2aLimit ? 10.0f : 3.0f;

        float norm = juce::jlimit(0.0f, 1.0f,
            (sidechainDb + 60.0f) / 60.0f);
        float chargeCoeff = la2aChargeCoeffSlow
            + norm * (la2aChargeCoeffFast - la2aChargeCoeffSlow);

        if (sidechain > optoFast)
            optoFast = chargeCoeff * optoFast + (1.0f - chargeCoeff) * sidechain;
        else
            optoFast = la2aFastDecayCoeff * optoFast
                     + (1.0f - la2aFastDecayCoeff) * sidechain;

        if (sidechain > optoSlow)
            optoSlow = chargeCoeff * optoSlow + (1.0f - chargeCoeff) * sidechain;
        else
            optoSlow = slowDecayCoeff * optoSlow
                     + (1.0f - slowDecayCoeff) * sidechain;

        float optoState = 0.6f * optoFast + 0.4f * optoSlow;

        if (std::abs(envelope) > 1.0f)
            compressionDuration += 1.0f / static_cast<float>(sr);
        else
            compressionDuration *= 0.999f;
        compressionDuration = std::min(compressionDuration, 10.0f);

        float optoDb = juce::Decibels::gainToDecibels(optoState, -96.0f);

        // Use local ratio - no member mutation
        float targetGr = computeGainReductionWith(
            optoDb, la2aRatio, kneeDb);

        float rCoeff = (std::abs(envelope) > 6.0f)
            ? slowDecayCoeff : la2aFastDecayCoeff;

        if (targetGr < envelope)
            envelope = la2aFixedAttackCoeff * envelope
                     + (1.0f - la2aFixedAttackCoeff) * targetGr;
        else
            envelope = rCoeff * envelope + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    float calcFET1176(float sidechainDb)
    {
        float srf = static_cast<float>(sr);
        float targetGr;

        if (allButtonsIn)
        {
            float grDepth    = std::abs(envelope);
            float allInRatio = 12.0f + grDepth * 0.8f;
            float savedKnee  = kneeDb;
            // Use local ratio - no member mutation needed here
            // because computeGainReductionWith accepts explicit params
            float wideKnee = 12.0f;
            targetGr = computeGainReductionWith(
                sidechainDb, allInRatio, wideKnee);
            (void)savedKnee;
        }
        else
        {
            targetGr = computeGainReduction(sidechainDb);
        }

        float normAttack   = (attackMs  - 0.1f)  / 99.9f;
        float normRelease  = (releaseMs - 10.0f) / 1990.0f;
        float mappedAttack  = 0.02f * std::pow(40.0f, normAttack);
        float mappedRelease = 50.0f * std::pow(22.0f, normRelease);
        float fetAttack  = AnalogMath::msToCoeff(mappedAttack,  srf);
        float fetRelease = AnalogMath::msToCoeff(mappedRelease, srf);

        if (targetGr < envelope)
            envelope = fetAttack  * envelope + (1.0f - fetAttack)  * targetGr;
        else
            envelope = fetRelease * envelope + (1.0f - fetRelease) * targetGr;

        return envelope;
    }

    float calcAPI2500(float rawSidechain, float releaseCoeff)
    {
        float srf = static_cast<float>(sr);

        // P0 FIX - Thrust HPF now uses per-channel state
        // applyThrustHPF called with channel index
        // For linked stereo sidechain we use ch=0 for the max value
        float sidechain = thrustOn
            ? applyThrustHPF(rawSidechain, 0)
            : rawSidechain;

        float sidechainDb = juce::Decibels::gainToDecibels(
            std::abs(sidechain), -96.0f);
        float targetGr = computeGainReduction(sidechainDb);
        float apiAttack = AnalogMath::msToCoeff(attackMs * 0.8f, srf);
        if (targetGr < envelope)
            envelope = apiAttack  * envelope + (1.0f - apiAttack)  * targetGr;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * targetGr;
        return envelope;
    }

    void calcThrustHPF(float freq)
    {
        double w0    = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * 0.707);
        double b0 = (1.0 + cosw0) / 2.0;
        double b1 = -(1.0 + cosw0);
        double b2 = (1.0 + cosw0) / 2.0;
        double a0 =  1.0 + alpha;
        double a1 = -2.0 * cosw0;
        double a2 =  1.0 - alpha;
        thrustB0 = b0/a0; thrustB1 = b1/a0;
        thrustB2 = b2/a0; thrustA1 = a1/a0; thrustA2 = a2/a0;
    }

    // P0 FIX - per-channel Thrust HPF
    // ch parameter selects which state array to use
    float applyThrustHPF(float x, int ch)
    {
        double y = thrustB0 * x
                 + thrustB1 * thrustX1[ch]
                 + thrustB2 * thrustX2[ch]
                 - thrustA1 * thrustY1[ch]
                 - thrustA2 * thrustY2[ch];
        thrustX2[ch] = thrustX1[ch]; thrustX1[ch] = x;
        thrustY2[ch] = thrustY1[ch]; thrustY1[ch] = y;
        return static_cast<float>(y);
    }

    float applyModelColor(float x)
    {
        switch (model)
        {
            case Model::FAIRCHILD:
                return safeTanh(x * 1.1f) / 1.08f;
            case Model::LA2A:
                return x + 0.015f * x * x
                     * (x > 0.0f ? 1.0f : -1.0f);
            case Model::FET_1176:
            {
                float colored = x + 0.02f * x * x * x;
                if (allButtonsIn)
                    colored += 0.06f * x * x * x;
                return colored;
            }
            // SSL and API are intentionally clean passthrough
            case Model::SSL_BUS:
            case Model::API_2500:
            default:
                return x;
        }
    }
};