#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>

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

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        envelope        = 0.0f;
        optoState       = 0.0f;
        fairchildEnergy = 0.0f;
        compressionDuration = 0.0f;

        // API Thrust sidechain HPF
        // 2nd order Butterworth at 150Hz
        calcThrustHPF(150.0f);
        thrustX1 = thrustX2 = thrustY1 = thrustY2 = 0.0f;
    }

    void setModel    (int m)   { model = static_cast<Model>(m); }
    void setThreshold(float t) { threshold = t; }
    void setRatio    (float r) { ratio = r; }
    void setAttack   (float a) { attackMs = a; }
    void setRelease  (float r) { releaseMs = r; }
    void setMix      (float m) { mix = m; }
    void setKnee     (float k) { kneeDb = k; }

    // P1 FIX - Makeup gain now uses smoothed value
    // passed in from processor to avoid zipper noise
    void setMakeupSmoothed(
        juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>& smoother)
    {
        makeupSmootherPtr = &smoother;
    }

    float getGainReduction() const
    {
        return currentGainReductionDb.load();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        float attackCoeff  = calculateCoeff(attackMs);
        float releaseCoeff = calculateCoeff(releaseMs);

        for (int i = 0; i < numSamples; i++)
        {
            // Get smoothed makeup gain this sample
            float makeup = (makeupSmootherPtr != nullptr)
                ? makeupSmootherPtr->getNextValue()
                : 1.0f;

            // Detect level across all channels
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
                        sidechain,
                        attackCoeff,
                        releaseCoeff);
                    break;
            }

            currentGainReductionDb.store(
                gainReductionDb);

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
        }
    }

private:
    double sr         = 44100.0;
    Model  model      = Model::SSL_BUS;
    float  threshold  = -20.0f;
    float  ratio      = 4.0f;
    float  attackMs   = 10.0f;
    float  releaseMs  = 100.0f;
    float  mix        = 1.0f;
    float  kneeDb     = 6.0f;

    // P2 FIX - Single linked stereo envelope
    // envelope[1] was never used - removed
    float envelope = 0.0f;

    float optoState           = 0.0f;
    float fairchildEnergy     = 0.0f;
    float compressionDuration = 0.0f;

    // API Thrust HPF state
    double thrustB0 = 1.0, thrustB1 = 0.0;
    double thrustB2 = 0.0, thrustA1 = 0.0;
    double thrustA2 = 0.0;
    double thrustX1 = 0.0, thrustX2 = 0.0;
    double thrustY1 = 0.0, thrustY2 = 0.0;

    // Smoothed makeup pointer from processor
    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>* 
        makeupSmootherPtr = nullptr;

    std::atomic<float> currentGainReductionDb{ 0.0f };

    float calculateCoeff(float timeMs)
    {
        if (timeMs <= 0.0f) return 0.0f;
        return std::exp(-1.0f /
            (static_cast<float>(sr) * timeMs * 0.001f));
    }

    float computeGainReduction(float inputDb)
    {
        float halfKnee  = kneeDb * 0.5f;
        float overshoot = inputDb - threshold;

        if (overshoot < -halfKnee)
            return 0.0f;

        if (overshoot < halfKnee)
        {
            float x = (overshoot + halfKnee) / kneeDb;
            overshoot = x * x * halfKnee;
        }

        return -(overshoot * (1.0f - 1.0f / ratio));
    }

    // SSL BUS - VCA
    // P2 FIX - Added auto-release behavior
    // Release time scales with gain reduction depth
    float calcSSL(float sidechainDb,
                  float attackCoeff,
                  float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);

        // Auto-release: release time scales with GR depth
        float grAbs = std::abs(envelope);
        float autoRelease = releaseMs
                          * (0.5f + grAbs / 20.0f);
        autoRelease = std::clamp(autoRelease,
                                 releaseMs * 0.3f,
                                 releaseMs * 3.0f);
        float adaptiveRelease = calculateCoeff(autoRelease);

        if (targetGr < envelope)
            envelope = attackCoeff * envelope
                     + (1.0f - attackCoeff) * targetGr;
        else
            envelope = adaptiveRelease * envelope
                     + (1.0f - adaptiveRelease) * targetGr;

        return envelope;
    }

    // FAIRCHILD 670 - Vari-mu tube
    // P2 FIX - Program dependent time constants
    // Fixed attack (~0.2ms), variable release
    // The 6-position time constant behavior
    float calcFairchild(float sidechainDb,
                        float sidechain)
    {
        float energyCoeff = calculateCoeff(300.0f);
        fairchildEnergy   = energyCoeff * fairchildEnergy
                          + (1.0f - energyCoeff)
                          * sidechain * sidechain;

        float targetGr = computeGainReduction(sidechainDb);

        // Fairchild attack is always fast (~0.2ms)
        // User attack knob maps to 6 positions
        // We use attackMs but clamp it to authentic range
        float fairchildAttack = std::clamp(attackMs,
                                            0.1f, 0.5f);
        float aCoeff = calculateCoeff(fairchildAttack);

        // Release gets slower the more we compress
        // and slower with sustained program energy
        float grDepth = std::abs(envelope);
        float dynamicRelease = releaseMs
                             + grDepth * 500.0f
                             + fairchildEnergy * 200.0f;
        float rCoeff = calculateCoeff(dynamicRelease);

        if (targetGr < envelope)
            envelope = aCoeff * envelope
                     + (1.0f - aCoeff) * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    // LA-2A - Optical
    // P2 FIX - Fixed attack physics
    // Two stage release with photon memory
    float calcLA2A(float sidechainDb, float sidechain)
    {
        // Fixed opto charge time - not user adjustable
        float chargeCoeff    = calculateCoeff(10.0f);
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

        // CdS photon memory effect
        // Longer compression = slower release tail
        if (std::abs(envelope) > 1.0f)
            compressionDuration += 
                1.0f / static_cast<float>(sr);
        else
            compressionDuration *= 0.999f;

        compressionDuration = std::min(
            compressionDuration, 10.0f);

        // Two stage release
        float fastMs  = 40.0f;
        float slowMs  = 500.0f
                      + compressionDuration * 2000.0f;
        slowMs = std::min(slowMs, 5000.0f);

        float fastCoeff = calculateCoeff(fastMs);
        float slowCoeff = calculateCoeff(slowMs);

        // Fixed attack - LA-2A has no user attack
        float fixedAttack = calculateCoeff(10.0f);

        float rCoeff = (std::abs(envelope) > 6.0f)
            ? slowCoeff : fastCoeff;

        if (targetGr < envelope)
            envelope = fixedAttack * envelope
                     + (1.0f - fixedAttack) * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    // 1176 FET
    // P2 FIX - Attack range corrected
    // Real 1176 attack: 0.02ms to 0.8ms
    float calcFET1176(float sidechainDb,
                      float attackCoeff,
                      float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);

        // Map user attack (0.1-100ms) to 1176 range
        // (0.02ms to 0.8ms) logarithmically
        float normalizedAttack = (attackMs - 0.1f) / 99.9f;
        float mappedAttack = 0.02f
                           * std::pow(40.0f,
                               normalizedAttack);
        float fetAttack = calculateCoeff(mappedAttack);

        float grDepth = std::abs(envelope);
        float dynamicRelease = releaseMs
                             * (1.0f - grDepth * 0.02f);
        dynamicRelease = std::max(10.0f, dynamicRelease);
        float fetRelease = calculateCoeff(dynamicRelease);

        if (targetGr < envelope)
            envelope = fetAttack  * envelope
                     + (1.0f - fetAttack)  * targetGr;
        else
            envelope = fetRelease * envelope
                     + (1.0f - fetRelease) * targetGr;

        return envelope;

        (void)attackCoeff;
        (void)releaseCoeff;
    }

    // API 2500 - VCA with Thrust circuit
    // P2 FIX - Thrust is now a real sidechain HPF
    // Not a threshold offset (previous was wrong)
    float calcAPI2500(float rawSidechain,
                      float attackCoeff,
                      float releaseCoeff)
    {
        // Apply Thrust HPF to sidechain
        // Bass does not trigger compression
        float filtered = applyThrustHPF(rawSidechain);

        float sidechainDb =
            juce::Decibels::gainToDecibels(
                std::abs(filtered), -96.0f);

        float targetGr = computeGainReduction(sidechainDb);

        float apiAttack = calculateCoeff(attackMs * 0.8f);

        if (targetGr < envelope)
            envelope = apiAttack  * envelope
                     + (1.0f - apiAttack)  * targetGr;
        else
            envelope = releaseCoeff * envelope
                     + (1.0f - releaseCoeff) * targetGr;

        return envelope;
    }

    // API Thrust HPF - 2nd order Butterworth at 150Hz
    // Filters the sidechain so bass does not
    // trigger the compressor
    void calcThrustHPF(float freq)
    {
        double w0 = 2.0 * juce::MathConstants<double>::pi
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

        thrustB0 = b0 / a0;
        thrustB1 = b1 / a0;
        thrustB2 = b2 / a0;
        thrustA1 = a1 / a0;
        thrustA2 = a2 / a0;
    }

    float applyThrustHPF(float x)
    {
        double y = thrustB0 * x
                 + thrustB1 * thrustX1
                 + thrustB2 * thrustX2
                 - thrustA1 * thrustY1
                 - thrustA2 * thrustY2;
        thrustX2 = thrustX1;
        thrustX1 = x;
        thrustY2 = thrustY1;
        thrustY1 = y;
        return static_cast<float>(y);
    }

    // Model specific output coloration
    float applyModelColor(float x)
    {
        switch (model)
        {
            case Model::FAIRCHILD:
                return std::tanh(x * 1.1f) / 1.08f;

            case Model::LA2A:
                return x + 0.015f * x * x
                     * (x > 0.0f ? 1.0f : -1.0f);

            case Model::FET_1176:
                return x + 0.02f * x * x * x;

            case Model::SSL_BUS:
            case Model::API_2500:
            default:
                return x;
        }
    }
};