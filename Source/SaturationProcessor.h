#pragma once
#include <JuceHeader.h>
#include <cmath>

class SaturationProcessor
{
public:

    void prepare(double sampleRate)
    {
        sr = sampleRate;
        for (int ch = 0; ch < 2; ch++)
        {
            dcPrevIn[ch]  = 0.0f;
            dcPrevOut[ch] = 0.0f;
        }
    }

    void process(juce::AudioBuffer<float>& buffer,
                 float drive,
                 float mix,
                 int   model)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float dry    = data[i];
                float output = dry;

                //──────────────────────────────────
                // P2 FIX - DC blocker always runs
                // Even when drive is zero
                // Prevents click when drive rises
                //──────────────────────────────────
                if (drive >= 0.001f)
                {
                    float preGain = 1.0f + drive * 9.0f;
                    float pushed  = dry * preGain;

                    float saturated = applySaturation(
                        pushed, model);

                    //──────────────────────────────
                    // P2 FIX - Level compensation
                    // Measure actual ceiling of the
                    // saturation function at this drive
                    // Normalize output against it
                    // Prevents -20dB drop at high drive
                    //──────────────────────────────
                    float ceiling = applySaturation(
                        preGain, model);
                    float compensated = (ceiling > 0.001f)
                        ? saturated / ceiling
                        : saturated;

                    output = dry * (1.0f - mix)
                           + compensated * mix;
                }

                // DC blocker always runs
                float dcOut = output
                            - dcPrevIn[ch]
                            + 0.9995f * dcPrevOut[ch];
                dcPrevIn[ch]  = output;
                dcPrevOut[ch] = dcOut;
                data[i] = dcOut;
            }
        }
    }

private:
    double sr = 44100.0;

    float dcPrevIn[2]  = { 0.0f, 0.0f };
    float dcPrevOut[2] = { 0.0f, 0.0f };

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
    // Asymmetric - DC blocker handles offset
    float neve(float x)
    {
        float bias   = 0.15f;
        float biased = x + bias;

        float y;
        if (biased >= 0.0f)
            y = std::tanh(biased * 1.2f);
        else
            y = std::tanh(biased * 1.8f) * 0.85f;

        // Second harmonic injection
        y += 0.08f * x * x;

        // P2 FIX - No inline DC correction
        // Let the DC blocker downstream handle it
        // Arbitrary constant was leaving DC residual
        return y;
    }

    // SSL - VCA
    // Symmetric, clean, odd harmonics
    float ssl(float x)
    {
        float y = std::tanh(x * 1.1f);
        y += 0.02f * x * x * x;
        return y;
    }

    // API - Discrete op-amp
    // Clean linear region, hard knee above
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
                std::tanh((absx - 0.6f) * 3.0f)
                * 0.35f);
        }
        return y;
    }

    // TUBE - 12AX7 triode
    // Strong even harmonics, asymmetric
    // Exponential approximation of transconductance
    float tube(float x)
    {
        float bias   = 0.25f;
        float biased = x + bias;

        float y;
        if (biased >= 0.0f)
        {
            // Soft ceiling - plate saturation
            y = 1.0f - std::exp(-biased * 1.5f);
        }
        else
        {
            // Hard floor - tube cutoff
            y = -(1.0f - std::exp(biased * 2.5f));
        }

        // Strong 2nd harmonic
        y += 0.15f * x * x;

        return y * 0.85f;
    }

    // TAPE - Magnetic oxide
    // Arctangent is mathematically correct for tape
    // Symmetric, gentle onset, soft ceiling
    float tape(float x)
    {
        float y = (2.0f / juce::MathConstants<float>::pi)
                * std::atan(
                    x * juce::MathConstants<float>::pi
                    * 0.5f);

        // Subtle intermodulation character
        // Gentle - not the aliasing sin() term
        y += 0.015f * x * std::abs(x);

        return y;
    }

    // FET - JFET input stage
    // Hard knee, fast response
    // P2 FIX - Removed sin(excess*30) aliasing term
    float fet(float x)
    {
        float threshold = 0.7f;

        if (std::abs(x) <= threshold)
        {
            return x + 0.05f * x * x * x;
        }
        else
        {
            float sign   = (x > 0.0f) ? 1.0f : -1.0f;
            float absx   = std::abs(x);
            float excess = absx - threshold;

            // Clean hard clip with tanh transition
            // Aliasing sin() term removed
            float clipped = threshold
                          + std::tanh(excess * 4.0f)
                          * (1.0f - threshold);

            return sign * clipped;
        }
    }

    // IRON - Output transformer only
    // Most subtle and transparent
    float iron(float x)
    {
        float y = std::tanh(x * 1.05f) / 1.02f;
        y += 0.03f * x * x
           * (x > 0.0f ? 1.0f : -1.0f);
        return y;
    }
};