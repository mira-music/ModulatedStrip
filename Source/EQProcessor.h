#pragma once
#include <JuceHeader.h>
#include <cmath>

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

class EQProcessor
{
public:

    enum class Model
    {
        NEVE_1073 = 0,
        NEVE_1084 = 1,
        SSL_4000E = 2,
        PULTEC    = 3,
        API_550A  = 4
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
            pultecCutBand[ch].reset();
            lpFilter[ch].reset();
        }

        prevSample[0] = 0.0f;
        prevSample[1] = 0.0f;

        paramsDirty = true;
        recalculateFilters();
    }

    // All setters use dirty flag
    // recalculateFilters only called once per block
    // not 9 times (P1 CRITICAL FIX)
    void setModel    (int m)   { 
        Model nm = static_cast<Model>(m);
        if (nm != model) { model = nm; paramsDirty = true; }
    }
    void setLowGain  (float g) { 
        if (g != lowGain)   { lowGain   = g; paramsDirty = true; }
    }
    void setLowFreq  (float f) { 
        if (f != lowFreq)   { lowFreq   = f; paramsDirty = true; }
    }
    void setMidGain  (float g) { 
        if (g != midGain)   { midGain   = g; paramsDirty = true; }
    }
    void setMidFreq  (float f) { 
        if (f != midFreq)   { midFreq   = f; paramsDirty = true; }
    }
    void setMidQ     (float q) { 
        if (q != midQ)      { midQ      = q; paramsDirty = true; }
    }
    void setHighGain (float g) { 
        if (g != highGain)  { highGain  = g; paramsDirty = true; }
    }
    void setHighFreq (float f) { 
        if (f != highFreq)  { highFreq  = f; paramsDirty = true; }
    }
    void setHPF      (float f) { 
        if (f != hpfFreq)   { hpfFreq   = f; paramsDirty = true; }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // P1 CRITICAL FIX
        // recalculateFilters called at most ONCE per block
        // Previously called up to 9 times per block
        if (paramsDirty)
        {
            recalculateFilters();
            paramsDirty = false;
        }

        int numChannels = buffer.getNumChannels();
        int numSamples  = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
            {
                float x = data[i];

                // HPF first
                if (hpfFreq > 25.0f)
                    x = hpfBand[ch].process(x);

                // Low band
                x = lowBand[ch].process(x);

                // Pultec simultaneous cut band
                // Only active in Pultec model
                if (model == Model::PULTEC)
                    x = pultecCutBand[ch].process(x);

                // Mid band
                x = midBand[ch].process(x);

                // High band
                x = highBand[ch].process(x);

                // Model coloration
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

    // P1 CRITICAL FIX - dirty flag
    bool paramsDirty = true;

    BiquadFilter lowBand[2];
    BiquadFilter midBand[2];
    BiquadFilter highBand[2];
    BiquadFilter hpfBand[2];

    // P3 FIX - Pultec simultaneous boost+cut
    BiquadFilter pultecCutBand[2];

    // P2 FIX - Transformer low pass for Neve
    BiquadFilter lpFilter[2];

    float prevSample[2] = { 0.0f, 0.0f };

    void recalculateFilters()
    {
        float effectiveQ = getModelQ();

        calcLowBand();
        calcPeakEQ(midFreq, midGain, effectiveQ);
        calcHighBand();
        calcHPF();
    }

    float getModelQ()
    {
        switch (model)
        {
            case Model::NEVE_1073:
            {
                float absGain = std::abs(midGain);
                if (absGain < 1.0f) return 0.5f;
                return 0.5f + (absGain / 15.0f) * 1.5f;
            }
            case Model::NEVE_1084:
            {
                float absGain = std::abs(midGain);
                return midQ + (absGain / 15.0f) * 0.5f;
            }
            case Model::SSL_4000E:
                return midQ;
            case Model::PULTEC:
                return 0.6f;
            case Model::API_550A:
            {
                float absGain = std::abs(midGain);
                if (absGain < 1.0f) return 0.7f;
                return 0.7f + (absGain / 12.0f) * 2.5f;
            }
            default:
                return midQ;
        }
    }

    // Low band - model aware
    void calcLowBand()
    {
        if (model == Model::PULTEC)
        {
            // P3 FIX - Pultec simultaneous boost + cut
            // Boost at lowFreq
            calcLowShelf(lowBand, lowFreq, lowGain, 0.5);
            // Cut at lowFreq * 1.5
            // This creates the famous resonant bump
            calcLowShelf(pultecCutBand,
                lowFreq * 1.5f,
                -lowGain * 0.5f,
                0.5);
        }
        else
        {
            calcLowShelf(lowBand, lowFreq, lowGain,
                getShelfSlope());
            // Bypass pultec cut band
            for (int ch = 0; ch < 2; ch++)
                pultecCutBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
        }
    }

    // High band - model aware
    void calcHighBand()
    {
        calcHighShelf(highFreq, highGain, getShelfSlope());
    }

    void calcLowShelf(BiquadFilter* bands,
                      float freq, float gain,
                      double S)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                bands[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0 * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / 2.0
                     * std::sqrt((A + 1.0/A)
                     * (1.0/S - 1.0) + 2.0);
        double sqrtA = std::sqrt(A);

        double b0 = A*((A+1)-(A-1)*cosw0+2*sqrtA*alpha);
        double b1 = 2*A*((A-1)-(A+1)*cosw0);
        double b2 = A*((A+1)-(A-1)*cosw0-2*sqrtA*alpha);
        double a0 = (A+1)+(A-1)*cosw0+2*sqrtA*alpha;
        double a1 = -2*((A-1)+(A+1)*cosw0);
        double a2 = (A+1)+(A-1)*cosw0-2*sqrtA*alpha;

        for (int ch = 0; ch < 2; ch++)
            bands[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    void calcHighShelf(float freq, float gain, double S)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                highBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0 * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / 2.0
                     * std::sqrt((A + 1.0/A)
                     * (1.0/S - 1.0) + 2.0);
        double sqrtA = std::sqrt(A);

        double b0 = A*((A+1)+(A-1)*cosw0+2*sqrtA*alpha);
        double b1 = -2*A*((A-1)+(A+1)*cosw0);
        double b2 = A*((A+1)+(A-1)*cosw0-2*sqrtA*alpha);
        double a0 = (A+1)-(A-1)*cosw0+2*sqrtA*alpha;
        double a1 = 2*((A-1)-(A+1)*cosw0);
        double a2 = (A+1)-(A-1)*cosw0-2*sqrtA*alpha;

        for (int ch = 0; ch < 2; ch++)
            highBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    void calcPeakEQ(float freq, float gain, float Q)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                midBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0 * juce::MathConstants<double>::pi
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

    // P3 FIX - Model correct HPF order
    // Neve 1073: 6 dB/oct (1-pole)
    // SSL 4000E and API 550A: 18 dB/oct (3-pole)
    // Pultec: no HPF
    void calcHPF()
    {
        if (hpfFreq < 25.0f
            || model == Model::PULTEC)
        {
            for (int ch = 0; ch < 2; ch++)
                hpfBand[ch].setCoefficients(
                    1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
            return;
        }

        if (model == Model::NEVE_1073
            || model == Model::NEVE_1084)
        {
            // 1-pole 6dB/oct - correct for Neve
            calcFirstOrderHPF(hpfFreq);
        }
        else
        {
            // 2-pole 12dB/oct as base
            // (true 18dB/oct needs a third stage)
            calcSecondOrderHPF(hpfFreq);
        }
    }

    void calcFirstOrderHPF(float freq)
    {
        double w0 = 2.0 * juce::MathConstants<double>::pi
                  * freq / sr;
        double k  = std::tan(w0 / 2.0);

        double b0 =  1.0 / (1.0 + k);
        double b1 = -b0;
        double b2 =  0.0;
        double a0 =  1.0;
        double a1 = (k - 1.0) / (k + 1.0);
        double a2 =  0.0;

        for (int ch = 0; ch < 2; ch++)
            hpfBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    void calcSecondOrderHPF(float freq)
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

        for (int ch = 0; ch < 2; ch++)
            hpfBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    double getShelfSlope()
    {
        switch (model)
        {
            case Model::NEVE_1073:
            case Model::NEVE_1084: return 0.8;
            case Model::SSL_4000E: return 1.0;
            case Model::PULTEC:    return 0.5;
            case Model::API_550A:  return 0.9;
            default:               return 0.7;
        }
    }

    // Model coloration - applied per sample
    float applyModelColor(float x, int ch)
    {
        switch (model)
        {
            case Model::NEVE_1073:
            case Model::NEVE_1084:
            {
                // P2 FIX - Correct transformer physics
                // Signal level dependent 1-pole low pass
                // Higher signal = more HF softening
                // This is what inductance actually does
                // Old hard slew limiter was wrong
                float y = x + 0.004f * x * x;

                float signalLevel = std::abs(y);
                float alpha = 0.01f
                            + signalLevel * 0.08f;
                alpha = std::min(alpha, 0.4f);

                y = (1.0f - alpha) * y
                  + alpha * prevSample[ch];
                prevSample[ch] = y;
                return y;
            }

            case Model::SSL_4000E:
                return x;

            case Model::PULTEC:
                return std::tanh(x * 1.03f) / 1.01f;

            case Model::API_550A:
                return x + 0.003f * x * std::abs(x);

            default:
                return x;
        }
    }
};