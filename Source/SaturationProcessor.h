#pragma once
#include <JuceHeader.h>
#include <cmath>

class SaturationProcessor
{
public:

    // Called once when plugin loads
    void prepare(double sampleRate)
    {
        sr = sampleRate;

        // Reset DC blocking filter state
        // DC blocker removes any constant offset
        // that saturation can introduce
        for (int ch = 0; ch < 2; ch++)
        {
            dcPrevIn[ch]  = 0.0f;
            dcPrevOut[ch] = 0.0f;
        }
    }

    // Called every block from processBlock
    void process(juce::AudioBuffer<float>& buffer,
                 float drive,      // 0.0 to 1.0
                 float mix,        // 0.0 to 1.0
                 int   model)      // 0 to 6
    {
        // Nothing to do if drive is zero
        if (drive < 0.001f) return;

        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float dry = data[i];

                // How hard we push into saturation
                // drive 0-1 maps to gain of 1x to 10x
                float preGain = 1.0f + drive * 9.0f;

                // Push signal into saturation circuit
                float pushed = dry * preGain;

                // Apply the saturation curve
                float saturated = applySaturation(
                    pushed, model);

                // Bring level back down
                // Compensate for the preGain push
                float compensated = saturated / preGain;

                // Parallel blend
                // mix 0.0 = all dry
                // mix 1.0 = all saturated
                float output = dry * (1.0f - mix) 
                             + compensated * mix;

                // DC blocking filter
                // Removes any offset introduced by
                // asymmetric saturation curves
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

    // DC blocker state per channel
    float dcPrevIn[2]  = { 0.0f, 0.0f };
    float dcPrevOut[2] = { 0.0f, 0.0f };

    // Routes to the right saturation curve
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

    // ─────────────────────────────────────────
    // NEVE - Transformer character
    // Even harmonics dominant (2nd, 4th)
    // Warm, thick, musical
    // Asymmetric = different behavior + vs -
    // ─────────────────────────────────────────
    float neve(float x)
    {
        // Bias shifts the operating point
        // Creates asymmetry = even harmonics
        float bias = 0.15f;
        float biased = x + bias;

        float y;
        if (biased >= 0.0f)
            y = std::tanh(biased * 1.2f);
        else
            y = std::tanh(biased * 1.8f) * 0.85f;

        // Add subtle 2nd harmonic
        // x*x is always positive = 2nd harmonic shape
        y += 0.08f * x * x;

        return y - (bias * 0.3f);
    }

    // ─────────────────────────────────────────
    // SSL - VCA character
    // Clean with subtle odd harmonics
    // Tight, punchy, modern
    // Nearly symmetric = mostly odd harmonics
    // ─────────────────────────────────────────
    float ssl(float x)
    {
        // SSL is cleaner than Neve
        // Simple soft clip with very subtle color
        float y = std::tanh(x * 1.1f);

        // Very subtle 3rd harmonic
        // x*x*x = odd harmonic shape
        y += 0.02f * x * x * x;

        return y;
    }

    // ─────────────────────────────────────────
    // API - Discrete op-amp character
    // Forward, exciting, punchy
    // Mix of even and odd harmonics
    // ─────────────────────────────────────────
    float api(float x)
    {
        // API has a more aggressive knee
        // Stays cleaner in the middle
        // Clips harder at the edges
        float y;

        if (std::abs(x) < 0.6f)
        {
            // Clean region - minimal distortion
            y = x * (1.0f + 0.1f * x * x);
        }
        else
        {
            // Saturation region - harder clip
            float sign = (x > 0.0f) ? 1.0f : -1.0f;
            float absx = std::abs(x);
            y = sign * (0.6f + 
                std::tanh((absx - 0.6f) * 3.0f) 
                * 0.35f);
        }

        return y;
    }

    // ─────────────────────────────────────────
    // TUBE - 12AX7 triode character
    // Strong even harmonics
    // Smooth, vintage, musical
    // Most asymmetric of all models
    // ─────────────────────────────────────────
    float tube(float x)
    {
        // Tubes are very asymmetric
        // Positive and negative halves clip differently
        float bias = 0.25f;
        float biased = x + bias;

        float y;
        if (biased >= 0.0f)
        {
            // Positive half - softer
            y = 1.0f - std::exp(-biased * 1.5f);
        }
        else
        {
            // Negative half - harder
            y = -(1.0f - std::exp(biased * 2.5f));
        }

        // Strong 2nd harmonic (tube character)
        y += 0.15f * x * x;

        return (y - bias * 0.4f) * 0.85f;
    }

    // ─────────────────────────────────────────
    // TAPE - Magnetic tape character
    // Frequency dependent compression
    // Smooth symmetric saturation
    // Gentle, musical, warm
    // ─────────────────────────────────────────
    float tape(float x)
    {
        // Tape has a very smooth saturation
        // Using arctangent for a different curve shape
        // than tanh - slightly more gentle
        float y = (2.0f / juce::MathConstants<float>::pi)
                * std::atan(x * juce::MathConstants<float>::pi 
                * 0.5f);

        // Tape also has subtle intermodulation
        // This adds a very slight roughness
        y += 0.03f * std::sin(x * 8.0f) * std::abs(x);

        return y;
    }

    // ─────────────────────────────────────────
    // FET - Field Effect Transistor character
    // Aggressive odd harmonics
    // Fast, punchy, gritty
    // Hard knee, more aggressive
    // ─────────────────────────────────────────
    float fet(float x)
    {
        // FET has a hard knee
        // Clean below threshold, clips hard above
        float threshold = 0.7f;

        if (std::abs(x) <= threshold)
        {
            // Below threshold - mostly clean
            // Small amount of 3rd harmonic
            return x + 0.05f * x * x * x;
        }
        else
        {
            // Above threshold - hard clip with grit
            float sign  = (x > 0.0f) ? 1.0f : -1.0f;
            float absx  = std::abs(x);
            float excess = absx - threshold;

            // Hard clip base
            float clipped = threshold 
                          + std::tanh(excess * 4.0f) 
                          * (1.0f - threshold);

            // Add grit (high frequency harmonic content)
            clipped += sign * 0.05f 
                     * std::sin(excess * 30.0f) 
                     * excess;

            return sign * clipped;
        }
    }

    // ─────────────────────────────────────────
    // IRON - Output transformer only
    // Subtle, transparent coloration
    // LF bloom and HF softening
    // The most gentle option
    // ─────────────────────────────────────────
    float iron(float x)
    {
        // Iron core transformer
        // Very subtle saturation
        // Mostly just rounds the peaks gently
        float y = std::tanh(x * 1.05f) / 1.02f;

        // Subtle even harmonic
        y += 0.03f * x * x * (x > 0 ? 1.0f : -1.0f);

        return y;
    }
};