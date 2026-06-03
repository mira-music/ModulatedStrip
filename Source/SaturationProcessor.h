#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "AnalogMath.h"

class SaturationProcessor
{
public:

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        // Sample-rate-correct DC blocker
        // Ensures consistent 20Hz cutoff at all rates
        dcCoeff = std::exp(
            -2.0f * juce::MathConstants<float>::pi
            * 20.0f / static_cast<float>(sampleRate));

        for (int ch = 0; ch < 2; ch++)
        {
            dcPrevIn[ch]  = 0.0f;
            dcPrevOut[ch] = 0.0f;
        }
    }

    // Process using flat block values
    // Smoothers are advanced in PluginProcessor.cpp
    // using skip() to avoid the snapshot/freeze bug
    void process(juce::AudioBuffer<float>& buffer,
                 float drive,
                 float mix,
                 int   model)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        // Compute ceiling once per block
        float preGain    = 1.0f + drive * 9.0f;
        float ceiling    = (drive >= 0.001f)
            ? applySaturation(preGain, model)
            : 1.0f;
        float invCeiling = (ceiling > 0.001f)
            ? 1.0f / ceiling
            : 1.0f;

        // Only asymmetric models need DC blocking
        // NEVE, TUBE, IRON
        bool needsDC = (model == 0
                     || model == 3
                     || model == 6);

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float dry    = data[i];
                float output = dry;

                if (drive >= 0.001f)
                {
                    float pushed      = dry * preGain;
                    float saturated   = applySaturation(
                        pushed, model);
                    float compensated = saturated
                                      * invCeiling;

                    output = dry        * (1.0f - mix)
                           + compensated * mix;
                }

                if (needsDC)
                {
                    float dcOut = output
                                - dcPrevIn[ch]
                                + dcCoeff * dcPrevOut[ch];
                    dcPrevIn[ch]  = output;
                    dcPrevOut[ch] = dcOut;
                    data[i] = dcOut;
                }
                else
                {
                    data[i] = output;
                }
            }
        }
    }

private:
    double sr      = 44100.0;
    float  dcCoeff = 0.9995f;

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
    // Bounded second harmonic relative to output
    // Prevents the previous h2 overflow bug
    float neve(float x)
    {
        float bias   = 0.15f;
        float biased = x + bias;

        float y;
        if (biased >= 0.0f)
            y = AnalogMath::safeTanh(biased * 1.2f);
        else
            y = AnalogMath::safeTanh(biased * 1.8f)
              * 0.85f;

        // Bounded 2nd harmonic relative to output
        float yAbs = std::abs(y);
        float h2   = 0.06f * yAbs * y;
        y = y + h2;

        return y;
    }

    // SSL - VCA
    // Symmetric, clean, odd harmonics
    float ssl(float x)
    {
        float y = AnalogMath::safeTanh(x * 1.1f);
        y += 0.02f * x * x * x;
        return y;
    }

    // API - Discrete op-amp
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
            y = sign * (0.6f
                + AnalogMath::safeTanh(
                    (absx - 0.6f) * 3.0f)
                * 0.35f);
        }
        return y;
    }

    // TUBE - 12AX7 Child-Langmuir 3/2 power law
    float tube(float x)
    {
        float bias = 0.25f;
        float Vgk  = x + bias;

        // Hard cutoff
        if (Vgk < -0.8f) return -0.18f;

        float Vp      = 1.0f;
        float ip      = std::pow(
            std::max(0.0f, Vgk + Vp / 20.0f), 1.5f);
        float ipNorm  = std::min(ip, 1.0f);

        float y = (ipNorm - 0.5f) * 2.0f
                + 0.12f * x * x;

        return y * 0.85f;
    }

    // TAPE - Magnetic oxide
    // Arctangent mathematically correct for tape
    float tape(float x)
    {
        float y = (2.0f / juce::MathConstants<float>::pi)
                * std::atan(
                    x * juce::MathConstants<float>::pi
                    * 0.5f);

        y += 0.015f * x * std::abs(x);
        return y;
    }

    // FET - JFET
    float fet(float x)
    {
        const float threshold = 0.7f;

        if (std::abs(x) <= threshold)
            return x + 0.05f * x * x * x;

        float sign   = (x > 0.0f) ? 1.0f : -1.0f;
        float absx   = std::abs(x);
        float excess = absx - threshold;

        float clipped = threshold
                      + AnalogMath::safeTanh(excess * 4.0f)
                      * (1.0f - threshold);

        return sign * clipped;
    }

    // IRON - Output transformer
    float iron(float x)
    {
        float y = AnalogMath::safeTanh(x * 1.05f) / 1.02f;
        y += 0.03f * x * x
           * (x > 0.0f ? 1.0f : -1.0f);
        return y;
    }
};