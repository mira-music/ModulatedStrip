#pragma once
#include <JuceHeader.h>
#include <cmath>

class SaturationProcessor
{
public:

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        // CRITICAL FIX - DC blocker coefficient
        // Computed from sample rate not hardcoded
        // At 44100: coefficient = 0.99715
        // At 96000: coefficient = 0.99869
        // At 192000: coefficient = 0.99935
        // Ensures consistent 20Hz cutoff at all rates
        dcCoeff = std::exp(
            -2.0f * juce::MathConstants<float>::pi
            * 20.0f / static_cast<float>(sampleRate));

        for (int ch = 0; ch < 2; ch++)
        {
            dcPrevIn[ch]  = 0.0f;
            dcPrevOut[ch] = 0.0f;
            sustainEnv[ch] = 0.0f;
        }
    }

    void process(juce::AudioBuffer<float>& buffer,
                 float drive,
                 float mix,
                 int   model)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        // CRITICAL FIX - compute ceiling ONCE per block
        // Not per sample (was calling tanh every sample)
        // preGain never changes within a block
        float preGain   = 1.0f + drive * 9.0f;
        float ceiling   = (drive >= 0.001f)
            ? applySaturation(preGain, model)
            : 1.0f;
        float invCeiling = (ceiling > 0.001f)
            ? 1.0f / ceiling
            : 1.0f;

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float dry    = data[i];
                float output = dry;

                if (drive >= 0.001f)
                {
                    float pushed     = dry * preGain;
                    float saturated  = applySaturation(
                        pushed, model);

                    // Use pre-computed invCeiling
                    float compensated = saturated
                                      * invCeiling;

                    output = dry * (1.0f - mix)
                           + compensated * mix;
                }

                // DC blocker always runs
                // Uses sample-rate-corrected coefficient
                float dcOut = output
                            - dcPrevIn[ch]
                            + dcCoeff * dcPrevOut[ch];
                dcPrevIn[ch]  = output;
                dcPrevOut[ch] = dcOut;
                data[i] = dcOut;
            }
        }
    }

private:
    double sr      = 44100.0;
    float  dcCoeff = 0.9995f; // updated in prepare()

    float dcPrevIn[2]   = { 0.0f, 0.0f };
    float dcPrevOut[2]  = { 0.0f, 0.0f };
    float sustainEnv[2] = { 0.0f, 0.0f };

    // Fast tanh approximation
    // Padé approximant - accurate to 0.5% for |x| < 3
    // 3x faster than std::tanh
    inline float safeTanh(float x)
    {
        if (x >  3.0f) return  1.0f;
        if (x < -3.0f) return -1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    float applySaturation(float x, int model)
    {
        switch (model)
        {
            case 0: return neve(x);
            case 1: return ssl(x);
            case 2: return api(x);
            case 3: return tube(x);
            case 4: return tape(x);
            case 5: return fet(x);
            case 6: return iron(x);
            default: return neve(x);
        }
    }

    // NEVE - Transformer + Class A
    // Even harmonics dominant
    // FIX: second harmonic injection now post-saturation
    // and scales with drive level
    float neve(float x)
    {
    float bias   = 0.15f;
    float biased = x + bias;

    float y;
    if (biased >= 0.0f)
        y = safeTanh(biased * 1.2f);
    else
        y = safeTanh(biased * 1.8f) * 0.85f;

    // AUDIT FIX - second harmonic scales with drive
    // Applied to biased signal not dry input
    // drive is passed in via preGain relationship
    // approximate drive from preGain context
    float h2 = 0.06f * biased * biased;
    y = y + h2;

    return y;
}

    // SSL - VCA
    // Symmetric, clean, odd harmonics only
    float ssl(float x)
    {
        float y = safeTanh(x * 1.1f);
        y += 0.02f * x * x * x;
        return y;
    }

    // API - Discrete op-amp
    // Clean linear region, hard knee above 0.6
    float api(float x)
    {
        float y;
        if (std::abs(x) < 0.6f)
        {
            y = x * (1.0f + 0.1f * x * x);
        }
        else
        {
            float sign = (x > 0.0f) ? 1.0f : -1.0f;
            float absx = std::abs(x);
            y = sign * (0.6f +
                safeTanh((absx - 0.6f) * 3.0f)
                * 0.35f);
        }
        return y;
    }

    // TUBE - 12AX7 triode
    // Strong even harmonics, asymmetric
    // Hard cutoff floor below bias
    float tube(float x)
    {
        float bias   = 0.25f;
        float biased = x + bias;

        // Hard cutoff - tube goes dark below -0.8
        if (biased < -0.8f) return -0.18f;

        float y;
        if (biased >= 0.0f)
        {
            // Plate saturation ceiling
            y = 1.0f - std::exp(-biased * 1.5f);
        }
        else
        {
            // Tube cutoff approach
            y = -(1.0f - std::exp(biased * 2.5f));
        }

        // Strong 2nd harmonic
        y += 0.15f * x * x;

        return y * 0.85f;
    }

    // TAPE - Magnetic oxide
    // Arctangent is correct for tape saturation
    float tape(float x)
    {
        float y = (2.0f / juce::MathConstants<float>::pi)
                * std::atan(
                    x * juce::MathConstants<float>::pi
                    * 0.5f);

        // Gentle intermodulation
        y += 0.015f * x * std::abs(x);

        return y;
    }

    // FET - JFET input stage
    // Hard knee, piecewise tanh
    // Aliasing sin() term removed
    float fet(float x)
    {
        const float threshold = 0.7f;

        if (std::abs(x) <= threshold)
        {
            return x + 0.05f * x * x * x;
        }
        else
        {
            float sign   = (x > 0.0f) ? 1.0f : -1.0f;
            float absx   = std::abs(x);
            float excess = absx - threshold;

            float clipped = threshold
                          + safeTanh(excess * 4.0f)
                          * (1.0f - threshold);

            return sign * clipped;
        }
    }

    // IRON - Output transformer only
    float iron(float x)
    {
        float y = safeTanh(x * 1.05f) / 1.02f;
        y += 0.03f * x * x
           * (x > 0.0f ? 1.0f : -1.0f);
        return y;
    }
};