#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>

class CompressorProcessor
{
public:

    enum class Model
    {
        SSL_BUS     = 0,
        FAIRCHILD   = 1,
        LA2A        = 2,
        FET_1176    = 3,
        API_2500    = 4
    };

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        for (int ch = 0; ch < 2; ch++)
        {
            envelope[ch]   = 0.0f;
            gainSmooth[ch] = 1.0f;
        }

        optoState = 0.0f;
        fairchildEnergy = 0.0f;
    }

    void setModel    (int m)   { model = static_cast<Model>(m); }
    void setThreshold(float t) { threshold = t; }
    void setRatio    (float r) { ratio = r; }
    void setAttack   (float a) { attackMs = a; }
    void setRelease  (float r) { releaseMs = r; }
    void setMakeup   (float g) { makeupDb = g; }
    void setMix      (float m) { mix = m; }
    void setKnee     (float k) { kneeDb = k; }

    float getGainReduction() const 
    { 
        return currentGainReductionDb; 
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        float makeup = juce::Decibels::decibelsToGain(
            makeupDb);

        float attackCoeff  = calculateCoeff(attackMs);
        float releaseCoeff = calculateCoeff(releaseMs);

        for (int i = 0; i < numSamples; i++)
        {
            // Detect loudest level across channels
            float sidechain = 0.0f;
            for (int ch = 0; ch < numChannels; ch++)
            {
                float s = std::abs(
                    buffer.getSample(ch, i));
                sidechain = std::max(sidechain, s);
            }

            float sidechainDb = 
                juce::Decibels::gainToDecibels(
                    sidechain, -96.0f);

            // Calculate gain reduction based on model
            float gainReductionDb = 0.0f;

            switch (model)
            {
                case Model::SSL_BUS:
                    gainReductionDb = calcSSL(
                        sidechainDb, 
                        attackCoeff, 
                        releaseCoeff);
                    break;

                case Model::FAIRCHILD:
                    gainReductionDb = calcFairchild(
                        sidechainDb, 
                        sidechain);
                    break;

                case Model::LA2A:
                    gainReductionDb = calcLA2A(
                        sidechainDb, 
                        sidechain);
                    break;

                case Model::FET_1176:
                    gainReductionDb = calcFET1176(
                        sidechainDb,
                        attackCoeff,
                        releaseCoeff);
                    break;

                case Model::API_2500:
                    gainReductionDb = calcAPI2500(
                        sidechainDb,
                        attackCoeff,
                        releaseCoeff);
                    break;
            }

            currentGainReductionDb = gainReductionDb;

            float grLinear = 
                juce::Decibels::decibelsToGain(
                    gainReductionDb);

            // Apply to audio
            for (int ch = 0; ch < numChannels; ch++)
            {
                float dry = buffer.getSample(ch, i);
                float wet = dry * grLinear * makeup;
                wet = applyModelColor(wet, ch);

                float output = dry * (1.0f - mix) 
                             + wet * mix;

                buffer.setSample(ch, i, output);
            }
        }
    }

private:
    double sr         = 44100.0;
    Model  model      = Model::SSL_BUS;
    float  threshold  = -20.0f;
    float  ratio      = 4.0f;
    float  attackMs   = 10.0f;
    float  releaseMs  = 100.0f;
    float  makeupDb   = 0.0f;
    float  mix        = 1.0f;
    float  kneeDb     = 6.0f;

    float envelope[2]   = { 0.0f, 0.0f };
    float gainSmooth[2] = { 1.0f, 1.0f };

    float optoState       = 0.0f;
    float fairchildEnergy = 0.0f;

    std::atomic<float> currentGainReductionDb{ 0.0f };

    float calculateCoeff(float timeMs)
    {
        if (timeMs <= 0.0f) return 0.0f;
        return std::exp(-1.0f / 
            (static_cast<float>(sr) * timeMs * 0.001f));
    }

    float computeGainReduction(float inputDb)
    {
        float halfKnee = kneeDb * 0.5f;
        float overshoot = inputDb - threshold;

        if (overshoot < -halfKnee)
        {
            return 0.0f;
        }
        else if (overshoot < halfKnee)
        {
            float x = (overshoot + halfKnee) / kneeDb;
            overshoot = x * x * halfKnee;
        }

        return -(overshoot * (1.0f - 1.0f / ratio));
    }

    // SSL BUS - VCA, precise, punchy glue
    float calcSSL(float sidechainDb,
                  float attackCoeff,
                  float releaseCoeff)
    {
        float targetGr = computeGainReduction(
            sidechainDb);

        if (targetGr < envelope[0])
            envelope[0] = attackCoeff * envelope[0] 
                + (1.0f - attackCoeff) * targetGr;
        else
            envelope[0] = releaseCoeff * envelope[0] 
                + (1.0f - releaseCoeff) * targetGr;

        return envelope[0];
    }

    // FAIRCHILD 670 - Vari-mu tube
    // Release gets slower when compressing more
    float calcFairchild(float sidechainDb, 
                        float sidechain)
    {
        float energyCoeff = calculateCoeff(300.0f);
        fairchildEnergy = energyCoeff * fairchildEnergy 
            + (1.0f - energyCoeff) 
            * sidechain * sidechain;

        float targetGr = computeGainReduction(
            sidechainDb);

        float dynamicAttack = attackMs 
            + (1.0f - sidechain) * 50.0f;
        dynamicAttack = std::max(1.0f, dynamicAttack);
        float aCoeff = calculateCoeff(dynamicAttack);

        float grDepth = std::abs(envelope[0]);
        float dynamicRelease = releaseMs 
            + grDepth * 500.0f
            + fairchildEnergy * 200.0f;
        float rCoeff = calculateCoeff(dynamicRelease);

        if (targetGr < envelope[0])
            envelope[0] = aCoeff * envelope[0] 
                + (1.0f - aCoeff) * targetGr;
        else
            envelope[0] = rCoeff * envelope[0] 
                + (1.0f - rCoeff) * targetGr;

        return envelope[0];
    }

    // LA-2A - Optical, two stage release
    float calcLA2A(float sidechainDb, float sidechain)
    {
        float chargeCoeff = calculateCoeff(10.0f);
        float dischargeCoeff = calculateCoeff(
            releaseMs * 0.5f);

        if (sidechain > optoState)
            optoState = chargeCoeff * optoState 
                + (1.0f - chargeCoeff) * sidechain;
        else
            optoState = dischargeCoeff * optoState 
                + (1.0f - dischargeCoeff) * sidechain;

        float optoDb = juce::Decibels::gainToDecibels(
            optoState, -96.0f);
        float targetGr = computeGainReduction(optoDb);

        float fastRelease = calculateCoeff(60.0f);
        float slowRelease = calculateCoeff(
            std::max(releaseMs, 500.0f));

        float rCoeff = (std::abs(envelope[0]) > 6.0f) 
            ? slowRelease : fastRelease;

        float aCoeff = calculateCoeff(attackMs);

        if (targetGr < envelope[0])
            envelope[0] = aCoeff * envelope[0] 
                + (1.0f - aCoeff) * targetGr;
        else
            envelope[0] = rCoeff * envelope[0] 
                + (1.0f - rCoeff) * targetGr;

        return envelope[0];
    }

    // 1176 FET - Fastest attack, aggressive
    float calcFET1176(float sidechainDb,
                      float attackCoeff,
                      float releaseCoeff)
    {
        float targetGr = computeGainReduction(
            sidechainDb);

        float fetAttackCoeff = calculateCoeff(
            attackMs * 0.1f);

        float grDepth = std::abs(envelope[0]);
        float dynamicRelease = releaseMs 
            * (1.0f - grDepth * 0.02f);
        dynamicRelease = std::max(10.0f, dynamicRelease);
        float fetReleaseCoeff = calculateCoeff(
            dynamicRelease);

        if (targetGr < envelope[0])
            envelope[0] = fetAttackCoeff * envelope[0] 
                + (1.0f - fetAttackCoeff) * targetGr;
        else
            envelope[0] = fetReleaseCoeff * envelope[0] 
                + (1.0f - fetReleaseCoeff) * targetGr;

        return envelope[0];
    }

    // API 2500 - VCA with Thrust circuit
    float calcAPI2500(float sidechainDb,
                      float attackCoeff,
                      float releaseCoeff)
    {
        float thrustThreshold = threshold + 6.0f;
        float adjustedDb = sidechainDb;

        if (sidechainDb < thrustThreshold)
        {
            adjustedDb = threshold + 
                (sidechainDb - threshold) * 0.5f;
        }

        float targetGr = computeGainReduction(
            adjustedDb);

        float apiAttack = calculateCoeff(
            attackMs * 0.8f);

        if (targetGr < envelope[0])
            envelope[0] = apiAttack * envelope[0] 
                + (1.0f - apiAttack) * targetGr;
        else
            envelope[0] = releaseCoeff * envelope[0] 
                + (1.0f - releaseCoeff) * targetGr;

        return envelope[0];
    }

    // Model specific output coloration
    float applyModelColor(float x, int ch)
    {
        (void)ch;

        switch (model)
        {
            case Model::FAIRCHILD:
                return std::tanh(x * 1.1f) / 1.08f;

            case Model::LA2A:
                return x + 0.015f * x * x 
                    * (x > 0 ? 1.0f : -1.0f);

            case Model::FET_1176:
                return x + 0.02f * x * x * x;

            case Model::SSL_BUS:
            case Model::API_2500:
            default:
                return x;
        }
    }
};