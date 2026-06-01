#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// BIQUAD FILTER
// The building block of all EQ bands
// Each band uses one of these
// 5 multiplications + 4 additions per sample
// Extremely CPU efficient
//==============================================================================
class BiquadFilter
{
public:
    void reset()
    {
        x1 = x2 = y1 = y2 = 0.0;
    }

    void setCoefficients(double newB0, double newB1, 
                         double newB2, double newA0, 
                         double newA1, double newA2)
    {
        b0 = newB0 / newA0;
        b1 = newB1 / newA0;
        b2 = newB2 / newA0;
        a1 = newA1 / newA0;
        a2 = newA2 / newA0;
    }

    float process(float input)
    {
        double output = b0 * input 
                      + b1 * x1 
                      + b2 * x2 
                      - a1 * y1 
                      - a2 * y2;
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        return static_cast<float>(output);
    }

private:
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
};

//==============================================================================
// EQ PROCESSOR
// 3 band EQ with 5 analog models
// Low shelf + Mid peak + High shelf
// Each model has different Q curves and coloration
//==============================================================================
class EQProcessor
{
public:

    enum class Model
    {
        NEVE_1073  = 0,
        NEVE_1084  = 1,
        SSL_4000E  = 2,
        PULTEC     = 3,
        API_550A   = 4
    };

    void prepare(double sampleRate)
    {
        sr = sampleRate;

        for (int ch = 0; ch < 2; ch++)
        {
            lowBand[ch].reset();
            midBand[ch].reset();
            highBand[ch].reset();
            hpfBand[ch].reset();
        }

        prevSample[0] = 0.0f;
        prevSample[1] = 0.0f;

        recalculateFilters();
    }

    void setModel(int m)
    {
        Model newModel = static_cast<Model>(m);
        if (newModel != model)
        {
            model = newModel;
            recalculateFilters();
        }
    }

    void setLowGain(float g)
    {
        if (g != lowGain)
        {
            lowGain = g;
            recalculateFilters();
        }
    }

    void setLowFreq(float f)
    {
        if (f != lowFreq)
        {
            lowFreq = f;
            recalculateFilters();
        }
    }

    void setMidGain(float g)
    {
        if (g != midGain)
        {
            midGain = g;
            recalculateFilters();
        }
    }

    void setMidFreq(float f)
    {
        if (f != midFreq)
        {
            midFreq = f;
            recalculateFilters();
        }
    }

    void setMidQ(float q)
    {
        if (q != midQ)
        {
            midQ = q;
            recalculateFilters();
        }
    }

    void setHighGain(float g)
    {
        if (g != highGain)
        {
            highGain = g;
            recalculateFilters();
        }
    }

    void setHighFreq(float f)
    {
        if (f != highFreq)
        {
            highFreq = f;
            recalculateFilters();
        }
    }

    void setHPF(float f)
    {
        if (f != hpfFreq)
        {
            hpfFreq = f;
            recalculateFilters();
        }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float x = data[i];

                // High pass filter first
                if (hpfFreq > 25.0f)
                    x = hpfBand[ch].process(x);

                // Three band EQ
                x = lowBand[ch].process(x);
                x = midBand[ch].process(x);
                x = highBand[ch].process(x);

                // Model specific coloration
                x = applyModelColor(x, ch);

                data[i] = x;
            }
        }
    }

private:
    double sr = 44100.0;
    Model model = Model::NEVE_1073;

    float lowGain  = 0.0f;
    float lowFreq  = 100.0f;
    float midGain  = 0.0f;
    float midFreq  = 1000.0f;
    float midQ     = 1.0f;
    float highGain = 0.0f;
    float highFreq = 10000.0f;
    float hpfFreq  = 20.0f;

    // Two channels stereo
    BiquadFilter lowBand[2];
    BiquadFilter midBand[2];
    BiquadFilter highBand[2];
    BiquadFilter hpfBand[2];

    // For model coloration
    float prevSample[2] = { 0.0f, 0.0f };

    //──────────────────────────────────────────────
    // RECALCULATE ALL FILTERS
    // Called when any parameter changes
    // Not called per-sample (CPU efficient)
    //──────────────────────────────────────────────
    void recalculateFilters()
    {
        // Get model-specific Q for mid band
        float effectiveQ = getModelQ();

        // Calculate filter coefficients
        calcLowShelf(lowFreq, lowGain);
        calcPeakEQ(midFreq, midGain, effectiveQ);
        calcHighShelf(highFreq, highGain);
        calcHighPass(hpfFreq);
    }

    //──────────────────────────────────────────────
    // MODEL SPECIFIC Q BEHAVIOR
    // This is what makes each EQ sound different
    //──────────────────────────────────────────────
    float getModelQ()
    {
        switch (model)
        {
            case Model::NEVE_1073:
            {
                // Proportional Q
                // Wide at low gain, narrow at high gain
                // This is the secret of the Neve sound
                float absGain = std::abs(midGain);
                if (absGain < 1.0f) return 0.5f;
                return 0.5f + (absGain / 15.0f) * 2.5f;
            }

            case Model::NEVE_1084:
            {
                // 1084 has user adjustable Q
                // but still slightly proportional
                float absGain = std::abs(midGain);
                float baseQ = midQ;
                baseQ += (absGain / 15.0f) * 0.5f;
                return baseQ;
            }

            case Model::SSL_4000E:
            {
                // SSL is fully parametric
                // Q is exactly what user sets
                // No proportional behavior
                return midQ;
            }

            case Model::PULTEC:
            {
                // Pultec has fixed broad Q
                // Very wide and gentle
                return 0.6f;
            }

            case Model::API_550A:
            {
                // API has proportional Q
                // Similar to Neve but different curve
                float absGain = std::abs(midGain);
                if (absGain < 1.0f) return 0.7f;
                return 0.7f + (absGain / 12.0f) * 3.0f;
            }

            default:
                return midQ;
        }
    }

    //──────────────────────────────────────────────
    // LOW SHELF FILTER
    // Boosts or cuts everything below a frequency
    //──────────────────────────────────────────────
    void calcLowShelf(float freq, float gain)
    {
        if (std::abs(gain) < 0.01f)
        {
            // No gain = bypass (flat response)
            for (int ch = 0; ch < 2; ch++)
                lowBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A = std::pow(10.0, gain / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi 
                  * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);

        // Slope factor - model dependent
        double S = getShelfSlope();
        double alpha = sinw0 / 2.0 
                     * std::sqrt((A + 1.0/A) 
                     * (1.0/S - 1.0) + 2.0);

        double sqrtA = std::sqrt(A);

        double b0 = A * ((A+1) - (A-1)*cosw0 
                   + 2*sqrtA*alpha);
        double b1 = 2*A * ((A-1) - (A+1)*cosw0);
        double b2 = A * ((A+1) - (A-1)*cosw0 
                   - 2*sqrtA*alpha);
        double a0 = (A+1) + (A-1)*cosw0 
                   + 2*sqrtA*alpha;
        double a1 = -2 * ((A-1) + (A+1)*cosw0);
        double a2 = (A+1) + (A-1)*cosw0 
                   - 2*sqrtA*alpha;

        for (int ch = 0; ch < 2; ch++)
            lowBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    //──────────────────────────────────────────────
    // PEAK EQ FILTER (BELL)
    // Boosts or cuts around a specific frequency
    //──────────────────────────────────────────────
    void calcPeakEQ(float freq, float gain, float Q)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                midBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A = std::pow(10.0, gain / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi 
                  * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * Q);

        double b0 = 1.0 + alpha * A;
        double b1 = -2.0 * cosw0;
        double b2 = 1.0 - alpha * A;
        double a0 = 1.0 + alpha / A;
        double a1 = -2.0 * cosw0;
        double a2 = 1.0 - alpha / A;

        for (int ch = 0; ch < 2; ch++)
            midBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    //──────────────────────────────────────────────
    // HIGH SHELF FILTER
    // Boosts or cuts everything above a frequency
    //──────────────────────────────────────────────
    void calcHighShelf(float freq, float gain)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                highBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A = std::pow(10.0, gain / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi 
                  * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);

        double S = getShelfSlope();
        double alpha = sinw0 / 2.0 
                     * std::sqrt((A + 1.0/A) 
                     * (1.0/S - 1.0) + 2.0);

        double sqrtA = std::sqrt(A);

        double b0 = A * ((A+1) + (A-1)*cosw0 
                   + 2*sqrtA*alpha);
        double b1 = -2*A * ((A-1) + (A+1)*cosw0);
        double b2 = A * ((A+1) + (A-1)*cosw0 
                   - 2*sqrtA*alpha);
        double a0 = (A+1) - (A-1)*cosw0 
                   + 2*sqrtA*alpha;
        double a1 = 2 * ((A-1) - (A+1)*cosw0);
        double a2 = (A+1) - (A-1)*cosw0 
                   - 2*sqrtA*alpha;

        for (int ch = 0; ch < 2; ch++)
            highBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    //──────────────────────────────────────────────
    // HIGH PASS FILTER
    // Removes everything below a frequency
    // Used for cleaning up rumble
    //──────────────────────────────────────────────
    void calcHighPass(float freq)
    {
        if (freq < 25.0f)
        {
            for (int ch = 0; ch < 2; ch++)
                hpfBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double w0 = 2.0 * juce::MathConstants<double>::pi 
                  * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * 0.707);

        double b0 = (1.0 + cosw0) / 2.0;
        double b1 = -(1.0 + cosw0);
        double b2 = (1.0 + cosw0) / 2.0;
        double a0 = 1.0 + alpha;
        double a1 = -2.0 * cosw0;
        double a2 = 1.0 - alpha;

        for (int ch = 0; ch < 2; ch++)
            hpfBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    //──────────────────────────────────────────────
    // SHELF SLOPE
    // Different models have different shelf shapes
    //──────────────────────────────────────────────
    double getShelfSlope()
    {
        switch (model)
        {
            case Model::NEVE_1073:
                // Neve has a gentle resonant bump
                return 0.8;

            case Model::NEVE_1084:
                return 0.8;

            case Model::SSL_4000E:
                // SSL is cleaner, steeper shelf
                return 1.0;

            case Model::PULTEC:
                // Pultec has very gentle slope
                return 0.5;

            case Model::API_550A:
                return 0.9;

            default:
                return 0.7;
        }
    }

    //──────────────────────────────────────────────
    // MODEL SPECIFIC COLORATION
    // Each EQ model colors the signal differently
    //──────────────────────────────────────────────
    float applyModelColor(float x, int ch)
    {
        switch (model)
        {
            case Model::NEVE_1073:
            case Model::NEVE_1084:
            {
                // Neve: transformer saturation
                // Class A warmth, subtle even harmonic
                float y = x + 0.004f * x * x;

                // Inductor slew rate limiting
                float maxSlew = 0.4f;
                float diff = y - prevSample[ch];
                if (std::abs(diff) > maxSlew)
                    y = prevSample[ch] + maxSlew 
                      * (diff > 0 ? 1.0f : -1.0f);
                prevSample[ch] = y;
                return y;
            }

            case Model::SSL_4000E:
                // SSL is clean and precise
                return x;

            case Model::PULTEC:
                // Pultec tube output warmth
                return std::tanh(x * 1.03f) / 1.01f;

            case Model::API_550A:
                // API discrete op-amp coloration
                return x + 0.003f * x * std::abs(x);

            default:
                return x;
        }
    }
};