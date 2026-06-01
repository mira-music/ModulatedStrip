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

    // CRITICAL FIX - Fairchild 6-position time constants
    // Authentic hardware values from the manual
    struct FairchildTC
    {
        float attackMs;
        float releaseMs;
        bool  programDependent;
    };

    static constexpr FairchildTC fairchildPositions[6] = {
        { 0.2f,   300.0f, false },  // Position 1
        { 0.2f,   800.0f, false },  // Position 2
        { 0.4f,  2000.0f, false },  // Position 3
        { 0.4f,  5000.0f, true  },  // Position 4 (program)
        { 0.4f,  5000.0f, false },  // Position 5
        { 0.4f, 10000.0f, true  }   // Position 6 (program)
    };

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        // Reset all state
        resetState();

        // SSL 30Hz sidechain HPF
        calcSSLSidechainHPF();

        // API Thrust 150Hz sidechain HPF
        calcThrustHPF(150.0f);
    }

    // CRITICAL FIX - setModel resets state
    // Prevents model contamination during live switching
    void setModel(int m)
    {
        Model newModel = static_cast<Model>(m);
        if (newModel != model)
        {
            model = newModel;
            resetState();
        }
    }

    void setThreshold(float t) { threshold = t; }
    void setRatio    (float r) { ratio = r; }
    void setAttack   (float a) { attackMs = a; }
    void setRelease  (float r) { releaseMs = r; }
    void setMix      (float m) { mix = m; }
    void setKnee     (float k) { kneeDb = k; }

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

        // CRITICAL FIX - compute coefficients ONCE per block
        // Not per sample - exp() is expensive
        float attackCoeff  = calculateCoeff(attackMs);
        float releaseCoeff = calculateCoeff(releaseMs);

        for (int i = 0; i < numSamples; i++)
        {
            float makeup = (makeupSmootherPtr != nullptr)
                ? makeupSmootherPtr->getNextValue()
                : 1.0f;

            float gainReductionDb = 0.0f;

            switch (model)
            {
                case Model::SSL_BUS:
                {
                    // SSL uses RMS detection and summed L+R
                    // not peak max(L,R)
                    float sumSq = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                    {
                        float s = buffer.getSample(ch, i);
                        // Apply SSL 30Hz sidechain HPF
                        s = applySSLSidechainHPF(s, ch);
                        sumSq += s * s;
                    }
                    // RMS of summed channels
                    float rms = std::sqrt(
                        sumSq / static_cast<float>(
                            std::max(numChannels, 1)));
                    // Update RMS accumulator (~1ms window)
                    float rmsCoeff = 1.0f
                        - 1.0f / (0.001f
                        * static_cast<float>(sr));
                    rmsCoeff = juce::jlimit(0.0f, 0.999f,
                        rmsCoeff);
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
                    gainReductionDb = calcFairchild(
                        sidechainDb, sidechain);
                    break;
                }

                case Model::LA2A:
                {
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
                    gainReductionDb = calcLA2A(
                        sidechainDb, sidechain);
                    break;
                }

                case Model::FET_1176:
                {
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
                    gainReductionDb = calcFET1176(
                        sidechainDb,
                        attackCoeff,
                        releaseCoeff);
                    break;
                }

                case Model::API_2500:
                {
                    float sidechain = 0.0f;
                    for (int ch = 0; ch < numChannels; ch++)
                    {
                        float s = std::abs(
                            buffer.getSample(ch, i));
                        sidechain = std::max(sidechain, s);
                    }
                    gainReductionDb = calcAPI2500(
                        sidechain,
                        attackCoeff,
                        releaseCoeff);
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
        }
    }

private:
    double sr        = 44100.0;
    Model  model     = Model::SSL_BUS;
    float  threshold = -20.0f;
    float  ratio     = 4.0f;
    float  attackMs  = 10.0f;
    float  releaseMs = 100.0f;
    float  mix       = 1.0f;
    float  kneeDb    = 6.0f;

    // Single stereo-linked envelope
    float envelope = 0.0f;

    // LA-2A dual opto state
    float optoFast = 0.0f;
    float optoSlow = 0.0f;

    // Fairchild state
    float fairchildControl = 0.0f;

    // SSL state
    float sslRmsState            = 0.0f;
    float compressionActiveMs    = 0.0f;

    // LA-2A photon memory
    float compressionDuration = 0.0f;

    // SSL 30Hz sidechain HPF state
    double sslHpfB0 = 1.0, sslHpfB1 = 0.0;
    double sslHpfA1 = 0.0;
    double sslHpfX1[2] = { 0.0, 0.0 };
    double sslHpfY1[2] = { 0.0, 0.0 };

    // API Thrust HPF state
    double thrustB0 = 1.0, thrustB1 = 0.0;
    double thrustB2 = 0.0, thrustA1 = 0.0;
    double thrustA2 = 0.0;
    double thrustX1 = 0.0, thrustX2 = 0.0;
    double thrustY1 = 0.0, thrustY2 = 0.0;

    juce::SmoothedValue<float,
        juce::ValueSmoothingTypes::Linear>*
        makeupSmootherPtr = nullptr;

    std::atomic<float> currentGainReductionDb{ 0.0f };

    // CRITICAL FIX - reset all model state on switch
    void resetState()
    {
        envelope          = 0.0f;
        optoFast          = 0.0f;
        optoSlow          = 0.0f;
        fairchildControl  = 0.0f;
        sslRmsState       = 0.0f;
        compressionActiveMs = 0.0f;
        compressionDuration = 0.0f;

        // Reset HPF states
        for (int ch = 0; ch < 2; ch++)
        {
            sslHpfX1[ch] = 0.0;
            sslHpfY1[ch] = 0.0;
        }
        thrustX1 = thrustX2 = thrustY1 = thrustY2 = 0.0;
    }

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

        if (overshoot < halfKnee && kneeDb > 0.0f)
        {
            float x = (overshoot + halfKnee) / kneeDb;
            overshoot = x * x * halfKnee;
        }

        return -(overshoot * (1.0f - 1.0f / ratio));
    }

    // SSL 30Hz 1-pole HPF on sidechain
    void calcSSLSidechainHPF()
    {
        double wc = 2.0 * juce::MathConstants<double>::pi
                  * 30.0 / sr;
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

    // SSL BUS - VCA with RMS detection and auto-release
    float calcSSL(float sidechainDb,
                  float attackCoeff,
                  float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);

        // CRITICAL FIX - authentic SSL auto-release
        // State machine not continuous formula
        // Two capacitors switching at 160ms
        if (targetGr < 0.0f)
        {
            compressionActiveMs +=
                1000.0f / static_cast<float>(sr);
        }
        else
        {
            compressionActiveMs = 0.0f;
        }

        float releaseToUse;
        if (compressionActiveMs > 160.0f)
        {
            // Fast cap takes over after 160ms continuous
            // Typical fast cap time ~40ms
            releaseToUse = std::min(releaseMs, 40.0f);
        }
        else
        {
            releaseToUse = releaseMs;
        }

        float adaptiveRelease = calculateCoeff(
            releaseToUse);

        if (targetGr < envelope)
            envelope = attackCoeff  * envelope
                     + (1.0f - attackCoeff)  * targetGr;
        else
            envelope = adaptiveRelease * envelope
                     + (1.0f - adaptiveRelease) * targetGr;

        return envelope;
    }

    // FAIRCHILD 670 - Vari-mu tube
    // CRITICAL FIX - 6-position time constants
    // Attack clamped to authentic 0.2-0.4ms range
    float calcFairchild(float sidechainDb,
                        float sidechain)
    {
        // Map user knob to 6-position selector
        // attackMs range 0.1-100 maps to positions 1-6
        int position = static_cast<int>(
            (attackMs / 100.0f) * 5.0f);
        position = juce::jlimit(0, 5, position);

        const FairchildTC& tc = fairchildPositions[position];

        float aCoeff = calculateCoeff(tc.attackMs);

        float releaseToUse = tc.releaseMs;
        if (tc.programDependent)
        {
            // Program dependent: release gets slower
            // with deeper or longer compression
            float grDepth = std::abs(envelope);
            releaseToUse = tc.releaseMs
                         + grDepth * 1000.0f;
            releaseToUse = std::min(releaseToUse,
                                    10000.0f);
        }

        float rCoeff = calculateCoeff(releaseToUse);

        float targetGr = computeGainReduction(sidechainDb);

        if (targetGr < envelope)
            envelope = aCoeff * envelope
                     + (1.0f - aCoeff) * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    // LA-2A - Optical with dual opto cell physics
    // CRITICAL FIX - two separate opto components
    // Fast phosphor decay + slow CdS photon memory
    float calcLA2A(float sidechainDb, float sidechain)
    {
        // Level-dependent charge time
        // Higher levels charge faster (opto physics)
        float chargeMs = 20.0f
            - (sidechainDb + 60.0f) / 60.0f * 10.0f;
        chargeMs = juce::jlimit(10.0f, 20.0f, chargeMs);
        float chargeCoeff = calculateCoeff(chargeMs);

        // Fast component - phosphor decay (~40ms)
        float fastDecay = calculateCoeff(40.0f);
        float slowDecay = calculateCoeff(
            500.0f + compressionDuration * 2000.0f);
        slowDecay = calculateCoeff(
            std::min(500.0f
                   + compressionDuration * 2000.0f,
                     5000.0f));

        // Charge both components toward target
        if (sidechain > optoFast)
            optoFast = chargeCoeff * optoFast
                     + (1.0f - chargeCoeff) * sidechain;
        else
            optoFast = fastDecay * optoFast
                     + (1.0f - fastDecay) * sidechain;

        if (sidechain > optoSlow)
            optoSlow = chargeCoeff * optoSlow
                     + (1.0f - chargeCoeff) * sidechain;
        else
            optoSlow = slowDecay * optoSlow
                     + (1.0f - slowDecay) * sidechain;

        // Blend fast and slow components
        float optoState = 0.6f * optoFast
                        + 0.4f * optoSlow;

        // CdS photon memory - tracks compression duration
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

        // Fixed attack - no user control on real LA-2A
        float fixedAttack = calculateCoeff(10.0f);

        // Two-stage release from opto state
        float rCoeff = (std::abs(envelope) > 6.0f)
            ? slowDecay : fastDecay;

        if (targetGr < envelope)
            envelope = fixedAttack * envelope
                     + (1.0f - fixedAttack) * targetGr;
        else
            envelope = rCoeff * envelope
                     + (1.0f - rCoeff) * targetGr;

        return envelope;
    }

    // 1176 FET - Fastest attack
    // CRITICAL FIX - dead code removed
    // (void) lines were after return statement
    float calcFET1176(float sidechainDb,
                      float attackCoeff,
                      float releaseCoeff)
    {
        float targetGr = computeGainReduction(sidechainDb);

        // Map user attack (0.1-100ms) to 1176 range
        // (0.02ms to 0.8ms) logarithmically
        float normalizedAttack =
            (attackMs - 0.1f) / 99.9f;
        float mappedAttack = 0.02f
            * std::pow(40.0f, normalizedAttack);
        float fetAttack = calculateCoeff(mappedAttack);

        // Map user release (10-2000ms) to 1176 range
        // (50ms to 1100ms) logarithmically
        float normalizedRelease =
            (releaseMs - 10.0f) / 1990.0f;
        float mappedRelease = 50.0f
            * std::pow(22.0f, normalizedRelease);
        float fetRelease = calculateCoeff(mappedRelease);

        // CRITICAL FIX - (void) suppressions moved
        // before any usage, not after return
        (void)attackCoeff;
        (void)releaseCoeff;

        if (targetGr < envelope)
            envelope = fetAttack  * envelope
                     + (1.0f - fetAttack)  * targetGr;
        else
            envelope = fetRelease * envelope
                     + (1.0f - fetRelease) * targetGr;

        return envelope;
    }

    // API 2500 - VCA with Thrust sidechain HPF
    float calcAPI2500(float rawSidechain,
                      float attackCoeff,
                      float releaseCoeff)
    {
        // Apply Thrust HPF to sidechain
        float filtered = applyThrustHPF(rawSidechain);

        float sidechainDb =
            juce::Decibels::gainToDecibels(
                std::abs(filtered), -96.0f);

        float targetGr = computeGainReduction(sidechainDb);

        float apiAttack = calculateCoeff(
            attackMs * 0.8f);

        if (targetGr < envelope)
            envelope = apiAttack  * envelope
                     + (1.0f - apiAttack)  * targetGr;
        else
            envelope = releaseCoeff * envelope
                     + (1.0f - releaseCoeff) * targetGr;

        return envelope;
    }

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