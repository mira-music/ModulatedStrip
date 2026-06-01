#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// BIQUAD FILTER
// Double precision state for accuracy at low frequencies
//==============================================================================
class BiquadFilter
{
public:
    void reset()
    {
        x1 = x2 = y1 = y2 = 0.0;
    }

    void setCoefficients(double b0_, double b1_,
                         double b2_, double a0_,
                         double a1_, double a2_)
    {
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }

    // Set as unity (bypass)
    void setUnity()
    {
        setCoefficients(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    }

    float process(float input)
    {
        double out = b0 * input
                   + b1 * x1
                   + b2 * x2
                   - a1 * y1
                   - a2 * y2;
        x2 = x1; x1 = input;
        y2 = y1; y1 = out;
        return static_cast<float>(out);
    }

private:
    double b0=1, b1=0, b2=0;
    double a1=0, a2=0;
    double x1=0, x2=0, y1=0, y2=0;
};

//==============================================================================
// SVF STATE
// State Variable Filter for SSL mid band
// Produces the SSL bell shape with
// correct boost/cut asymmetry
//==============================================================================
struct SVFState
{
    float lp = 0.0f;
    float bp = 0.0f;
};

//==============================================================================
// EQ PROCESSOR
//==============================================================================
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
            hpfBand2[ch].reset();
            pultecCutBand[ch].reset();
            transformerHPF[ch].reset();
            lowResonance[ch].reset();
            lcAllpass[ch].reset();
            svfState[ch] = { 0.0f, 0.0f };

            // Slow envelope follower for Neve HF softening
            // Tracks sustained signal level ~500ms
            neveEnv[ch] = 0.0f;
        }

        paramsDirty = true;
        recalculateFilters();
    }

    // All setters use dirty flag
    void setModel(int m)
    {
        Model nm = static_cast<Model>(m);
        if (nm != model)
        {
            model = nm;
            paramsDirty = true;

            // Reset SVF state on model switch
            for (int ch = 0; ch < 2; ch++)
                svfState[ch] = { 0.0f, 0.0f };
        }
    }

    void setLowGain (float g)
    { if (g != lowGain)  { lowGain  = g; paramsDirty = true; } }
    void setLowFreq (float f)
    { if (f != lowFreq)  { lowFreq  = f; paramsDirty = true; } }
    void setMidGain (float g)
    { if (g != midGain)  { midGain  = g; paramsDirty = true; } }
    void setMidFreq (float f)
    { if (f != midFreq)  { midFreq  = f; paramsDirty = true; } }
    void setMidQ    (float q)
    { if (q != midQ)     { midQ     = q; paramsDirty = true; } }
    void setHighGain(float g)
    { if (g != highGain) { highGain = g; paramsDirty = true; } }
    void setHighFreq(float f)
    { if (f != highFreq) { highFreq = f; paramsDirty = true; } }
    void setHPF     (float f)
    { if (f != hpfFreq)  { hpfFreq  = f; paramsDirty = true; } }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // P1 FIX: recalculate at most once per block
        if (paramsDirty)
        {
            recalculateFilters();
            paramsDirty = false;
        }

        int numCh  = buffer.getNumChannels();
        int numSmp = buffer.getNumSamples();

        for (int ch = 0; ch < numCh; ch++)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSmp; i++)
            {
                float x = data[i];

                // 1. HPF (model-correct order)
                if (hpfFreq > 25.0f
                    && model != Model::PULTEC)
                {
                    x = hpfBand[ch].process(x);
                    // SSL gets extra 1-pole for 18dB/oct
                    if (model == Model::SSL_4000E)
                        x = hpfBand2[ch].process(x);
                }

                // 2. Neve always-on transformer HPF at 25Hz
                if (model == Model::NEVE_1073
                 || model == Model::NEVE_1084)
                {
                    x = transformerHPF[ch].process(x);
                }

                // 3. Low band
                x = lowBand[ch].process(x);

                // 4. Neve inductor resonance bloom
                if ((model == Model::NEVE_1073
                  || model == Model::NEVE_1084)
                  && lowGain > 2.0f)
                {
                    x = lowResonance[ch].process(x);
                }

                // 5. Pultec simultaneous cut band
                if (model == Model::PULTEC)
                    x = pultecCutBand[ch].process(x);

                // 6. Pultec LC allpass phase character
                if (model == Model::PULTEC
                 && lowGain > 0.0f)
                {
                    x = lcAllpass[ch].process(x);
                }

                // 7. Mid band
                // SSL uses SVF, all others use biquad
                if (model == Model::SSL_4000E)
                    x = processSVF(x, ch);
                else
                    x = midBand[ch].process(x);

                // 8. High band
                x = highBand[ch].process(x);

                // 9. Model coloration
                // ALWAYS runs - even at zero gain
                // This is what makes it a color EQ
                x = applyModelColor(x, ch);

                data[i] = x;
            }
        }
    }

private:
    double sr = 44100.0;
    Model  model = Model::NEVE_1073;

    float lowGain  = 0.0f;
    float lowFreq  = 100.0f;
    float midGain  = 0.0f;
    float midFreq  = 1000.0f;
    float midQ     = 1.0f;
    float highGain = 0.0f;
    float highFreq = 10000.0f;
    float hpfFreq  = 20.0f;

    bool paramsDirty = true;

    // Existing filters
    BiquadFilter lowBand[2];
    BiquadFilter midBand[2];
    BiquadFilter highBand[2];
    BiquadFilter hpfBand[2];
    BiquadFilter pultecCutBand[2];

    // New filters from audit
    BiquadFilter hpfBand2[2];      // SSL 3rd HPF pole
    BiquadFilter transformerHPF[2]; // Neve 25Hz fixed HPF
    BiquadFilter lowResonance[2];   // Neve inductor bloom
    BiquadFilter lcAllpass[2];      // Pultec phase bloom

    // SSL SVF state
    SVFState svfState[2];
    float    svfG  = 0.0f;
    float    svfK  = 0.0f;
    float    svfA1 = 0.0f;
    float    svfA2 = 0.0f;
    float    svfA3 = 0.0f;
    float    svfGain = 0.0f; // linear gain for SVF peak

    // Neve slow envelope for sustained HF softening
    float neveEnv[2] = { 0.0f, 0.0f };
    // Coefficient for ~500ms envelope
    // Updated in recalculateFilters()
    float neveEnvCoeff = 0.998f;

    // Pultec model state
    float prevSample[2] = { 0.0f, 0.0f };

    //──────────────────────────────────────────────
    // RECALCULATE ALL FILTERS
    // Called at most once per block via dirty flag
    //──────────────────────────────────────────────
    void recalculateFilters()
    {
        // Update Neve sustain envelope coefficient
        // ~500ms time constant for transformer magnetization
        neveEnvCoeff = std::exp(
            -1.0f / (static_cast<float>(sr) * 0.5f));

        float effectiveQ = getModelQ();

        calcLowBand();
        calcMidBand(effectiveQ);
        calcHighBand();
        calcHPF();
        calcNeveTransformerHPF();
        calcNeveInductorBloom();
        calcPultecAllpass();
        calcSSLSVF(effectiveQ);
    }

    //──────────────────────────────────────────────
    // MODEL Q - what makes each EQ sound different
    //──────────────────────────────────────────────
    float getModelQ()
    {
        switch (model)
        {
            case Model::NEVE_1073:
            {
                // Proportional Q - correct per audit
                // Q = 0.5 at 0dB, 2.0 at 15dB
                float absGain = std::abs(midGain);
                return 0.5f + (absGain / 15.0f) * 1.5f;
            }

            case Model::NEVE_1084:
            {
                // Slightly higher Q than 1073
                float absGain = std::abs(midGain);
                return midQ + (absGain / 15.0f) * 0.7f;
            }

            case Model::SSL_4000E:
                // Fixed Q - no proportional behavior
                // SSL is wider and more open than Neve
                return 1.0f;

            case Model::PULTEC:
                // Very broad - LC network character
                return 0.6f;

            case Model::API_550A:
            {
                // AUDIT FIX - quadratic growth
                // Most aggressive proportional Q of all models
                // At +2dB: Q≈0.7  At +12dB: Q≈3.5
                float absGain = std::abs(midGain);
                if (absGain < 1.0f) return 0.7f;
                float norm = absGain / 15.0f;
                return 0.7f + norm * norm * 4.3f;
            }

            default: return midQ;
        }
    }

    //──────────────────────────────────────────────
    // LOW BAND
    //──────────────────────────────────────────────
    void calcLowBand()
    {
        float gainToUse = lowGain;

        // AUDIT FIX - Neve 1073 boost-only low shelf
        // Hardware passive circuit constraint
        if (model == Model::NEVE_1073)
            gainToUse = std::max(0.0f, gainToUse);

        if (model == Model::PULTEC)
        {
            // AUDIT FIX - corrected multipliers
            // 1.41 (sqrt 2 = half octave above)
            // not 1.5 as was previously used
            // Cut gain ratio 0.4 not 0.5
            calcLowShelf(lowBand,
                lowFreq, gainToUse, 0.5);
            calcLowShelf(pultecCutBand,
                lowFreq * 1.41f,
                -gainToUse * 0.4f,
                0.7);
        }
        else
        {
            calcLowShelf(lowBand,
                lowFreq, gainToUse,
                getShelfSlope());

            for (int ch = 0; ch < 2; ch++)
                pultecCutBand[ch].setUnity();
        }
    }

    //──────────────────────────────────────────────
    // MID BAND
    //──────────────────────────────────────────────
    void calcMidBand(float Q)
    {
        // SSL SVF is calculated separately
        // Other models use standard biquad peak
        if (model != Model::SSL_4000E)
            calcPeakEQ(midFreq, midGain, Q);
    }

    //──────────────────────────────────────────────
    // HIGH BAND
    //──────────────────────────────────────────────
    void calcHighBand()
    {
        calcHighShelf(highFreq, highGain,
            getShelfSlope());
    }

    //──────────────────────────────────────────────
    // HPF - model correct order
    // Neve: 6dB/oct (1-pole)
    // SSL/API: 18dB/oct (cascade 12 + 6)
    // Pultec: none
    //──────────────────────────────────────────────
    void calcHPF()
    {
        if (hpfFreq < 25.0f
         || model == Model::PULTEC)
        {
            for (int ch = 0; ch < 2; ch++)
            {
                hpfBand[ch].setUnity();
                hpfBand2[ch].setUnity();
            }
            return;
        }

        if (model == Model::NEVE_1073
         || model == Model::NEVE_1084)
        {
            // 1-pole 6dB/oct - correct for Neve
            calcFirstOrderHPF(hpfBand, hpfFreq);
            for (int ch = 0; ch < 2; ch++)
                hpfBand2[ch].setUnity();
        }
        else
        {
            // 2-pole 12dB/oct base
            calcSecondOrderHPF(hpfBand, hpfFreq);

            // AUDIT FIX - SSL gets extra 1-pole
            // = 18dB/oct total
            if (model == Model::SSL_4000E)
                calcFirstOrderHPF(hpfBand2, hpfFreq);
            else
                for (int ch = 0; ch < 2; ch++)
                    hpfBand2[ch].setUnity();
        }
    }

    //──────────────────────────────────────────────
    // NEVE TRANSFORMER HPF
    // Fixed 25Hz 1-pole always active on Neve models
    // Tightens the extreme sub-bass
    //──────────────────────────────────────────────
    void calcNeveTransformerHPF()
    {
        if (model == Model::NEVE_1073
         || model == Model::NEVE_1084)
        {
            calcFirstOrderHPF(transformerHPF, 25.0f);
        }
        else
        {
            for (int ch = 0; ch < 2; ch++)
                transformerHPF[ch].setUnity();
        }
    }

    //──────────────────────────────────────────────
    // NEVE INDUCTOR RESONANCE BLOOM
    // Small peak above low shelf frequency
    // Creates the "pillowy" Neve low end character
    //──────────────────────────────────────────────
    void calcNeveInductorBloom()
    {
        if ((model == Model::NEVE_1073
          || model == Model::NEVE_1084)
          && lowGain > 2.0f)
        {
            float bloomFreq = lowFreq * 2.5f;
            float bloomGain = lowGain * 0.15f;
            calcPeakFilter(lowResonance,
                bloomFreq, bloomGain, 3.0f);
        }
        else
        {
            for (int ch = 0; ch < 2; ch++)
                lowResonance[ch].setUnity();
        }
    }

    //──────────────────────────────────────────────
    // PULTEC LC ALLPASS
    // Phase bloom from LC network resonance
    // Adds the characteristic Pultec "roundness"
    //──────────────────────────────────────────────
    void calcPultecAllpass()
    {
        if (model == Model::PULTEC && lowGain > 0.0f)
        {
            double w0 = 2.0
                * juce::MathConstants<double>::pi
                * lowFreq / sr;
            double cosw  = std::cos(w0);
            double sinw  = std::sin(w0);
            double alpha = sinw / (2.0 * 1.0); // Q=1.0

            // 2nd order allpass coefficients
            // Note: b = reversed(a) - allpass property
            double b0 = 1.0 - alpha;
            double b1 = -2.0 * cosw;
            double b2 = 1.0 + alpha;
            double a0 = 1.0 + alpha;
            double a1 = -2.0 * cosw;
            double a2 = 1.0 - alpha;

            for (int ch = 0; ch < 2; ch++)
                lcAllpass[ch].setCoefficients(
                    b0, b1, b2, a0, a1, a2);
        }
        else
        {
            for (int ch = 0; ch < 2; ch++)
                lcAllpass[ch].setUnity();
        }
    }

    //──────────────────────────────────────────────
    // SSL SVF MID BAND
    // State Variable Filter - gives SSL bell shape
    // Boost and cut are not mirror images
    // This is the SSL 4000E character
    //──────────────────────────────────────────────
    void calcSSLSVF(float Q)
    {
        if (model != Model::SSL_4000E) return;

        float freq = midFreq;
        float w  = 2.0f * juce::MathConstants<float>::pi
                 * freq / static_cast<float>(sr);
        svfG  = std::tan(w * 0.5f);
        svfK  = 1.0f / Q;
        svfA1 = 1.0f / (1.0f + svfG * (svfG + svfK));
        svfA2 = svfG * svfA1;
        svfA3 = svfG * svfA2;

        svfGain = std::pow(10.0f, midGain / 40.0f);
    }

    float processSVF(float x, int ch)
    {
        if (std::abs(midGain) < 0.01f) return x;

        SVFState& s = svfState[ch];

        float v3 = x - s.lp;
        float v1 = svfA1 * s.bp + svfA2 * v3;
        float v2 = s.lp + svfA2 * s.bp + svfA3 * v3;

        s.bp = 2.0f * v1 - s.bp;
        s.lp = 2.0f * v2 - s.lp;

        // Peaking EQ from SVF bandpass output
        // (svfGain - 1/svfGain) creates asymmetry
        float peakOut = x + (svfGain - 1.0f / svfGain)
                      * svfK * 0.5f * v1;

        return peakOut;
    }

    //──────────────────────────────────────────────
    // MODEL COLORATION
    // AUDIT FIX - always runs even at zero gain
    // This is what makes it a color EQ
    //──────────────────────────────────────────────
    float applyModelColor(float x, int ch)
    {
        switch (model)
        {
            case Model::NEVE_1073:
            case Model::NEVE_1084:
            {
                // Even harmonic - transformer core
                float y = x + 0.004f * x * x;

                // AUDIT FIX - sustained envelope
                // for transformer magnetization
                // Not instantaneous signal level
                neveEnv[ch] = neveEnvCoeff
                            * neveEnv[ch]
                            + (1.0f - neveEnvCoeff)
                            * std::abs(y);

                // HF softening scales with sustained level
                float alpha = 0.005f
                            + neveEnv[ch] * 0.12f;
                alpha = std::min(alpha, 0.4f);

                y = (1.0f - alpha) * y
                  + alpha * prevSample[ch];
                prevSample[ch] = y;
                return y;
            }

            case Model::SSL_4000E:
                // Clean - no coloration at zero gain
                return x;

            case Model::PULTEC:
            {
                // AUDIT FIX - level dependent tube saturation
                // 12AU7 output stage
                // More 2nd harmonic at higher levels
                float drive = 1.0f + std::abs(x) * 0.4f;
                return std::tanh(x * drive) / drive;
            }

            case Model::API_550A:
            {
                // AUDIT FIX - add 3rd harmonic
                // 2520 op-amp generates both 2nd and 3rd
                // 3rd more pronounced than Neve
                float h2 = 0.004f * x * std::abs(x);
                float h3 = 0.006f * x * x * x;
                return x + h2 + h3;
            }

            default:
                return x;
        }
    }

    //──────────────────────────────────────────────
    // FILTER CALCULATIONS
    // All use Audio EQ Cookbook formulas
    //──────────────────────────────────────────────

    void calcLowShelf(BiquadFilter* bands,
                      float freq, float gain,
                      double S)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                bands[ch].setUnity();
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw  = std::cos(w0);
        double sinw  = std::sin(w0);
        double alpha = sinw / 2.0
                     * std::sqrt((A + 1.0/A)
                     * (1.0/S - 1.0) + 2.0);
        double sqA   = std::sqrt(A);

        double b0 = A*((A+1)-(A-1)*cosw+2*sqA*alpha);
        double b1 = 2*A*((A-1)-(A+1)*cosw);
        double b2 = A*((A+1)-(A-1)*cosw-2*sqA*alpha);
        double a0 = (A+1)+(A-1)*cosw+2*sqA*alpha;
        double a1 = -2*((A-1)+(A+1)*cosw);
        double a2 = (A+1)+(A-1)*cosw-2*sqA*alpha;

        for (int ch = 0; ch < 2; ch++)
            bands[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    void calcHighShelf(float freq, float gain, double S)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                highBand[ch].setUnity();
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw  = std::cos(w0);
        double sinw  = std::sin(w0);
        double alpha = sinw / 2.0
                     * std::sqrt((A + 1.0/A)
                     * (1.0/S - 1.0) + 2.0);
        double sqA   = std::sqrt(A);

        double b0 = A*((A+1)+(A-1)*cosw+2*sqA*alpha);
        double b1 = -2*A*((A-1)+(A+1)*cosw);
        double b2 = A*((A+1)+(A-1)*cosw-2*sqA*alpha);
        double a0 = (A+1)-(A-1)*cosw+2*sqA*alpha;
        double a1 = 2*((A-1)-(A+1)*cosw);
        double a2 = (A+1)-(A-1)*cosw-2*sqA*alpha;

        for (int ch = 0; ch < 2; ch++)
            highBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    void calcPeakEQ(float freq, float gain, float Q)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                midBand[ch].setUnity();
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw  = std::cos(w0);
        double sinw  = std::sin(w0);
        double alpha = sinw / (2.0 * Q);

        // Note: b1 == a1 for peaking EQ (always)
        double b0 = 1.0 + alpha * A;
        double b1 = -2.0 * cosw;
        double b2 = 1.0 - alpha * A;
        double a0 = 1.0 + alpha / A;
        double a1 = -2.0 * cosw;
        double a2 = 1.0 - alpha / A;

        for (int ch = 0; ch < 2; ch++)
            midBand[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    // Separate peak filter for bloom and resonance
    void calcPeakFilter(BiquadFilter* bands,
                        float freq, float gain, float Q)
    {
        if (std::abs(gain) < 0.01f)
        {
            for (int ch = 0; ch < 2; ch++)
                bands[ch].setUnity();
            return;
        }

        double A     = std::pow(10.0, gain / 40.0);
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw  = std::cos(w0);
        double sinw  = std::sin(w0);
        double alpha = sinw / (2.0 * Q);

        double b0 = 1.0 + alpha * A;
        double b1 = -2.0 * cosw;
        double b2 = 1.0 - alpha * A;
        double a0 = 1.0 + alpha / A;
        double a1 = -2.0 * cosw;
        double a2 = 1.0 - alpha / A;

        for (int ch = 0; ch < 2; ch++)
            bands[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    // 1-pole HPF (6dB/oct) - bilinear transform
    // Correct for Neve and transformer HPF
    void calcFirstOrderHPF(BiquadFilter* bands,
                           float freq)
    {
        double wc = 2.0
                  * juce::MathConstants<double>::pi
                  * freq / sr;
        double k  = std::tan(wc / 2.0);

        double b0 =  1.0 / (1.0 + k);
        double b1 = -b0;
        double b2 =  0.0;
        double a0 =  1.0;
        double a1 = (k - 1.0) / (k + 1.0);
        double a2 =  0.0;

        for (int ch = 0; ch < 2; ch++)
            bands[ch].setCoefficients(
                b0, b1, b2, a0, a1, a2);
    }

    // 2-pole Butterworth HPF (12dB/oct)
    void calcSecondOrderHPF(BiquadFilter* bands,
                            float freq)
    {
        double w0    = 2.0
                     * juce::MathConstants<double>::pi
                     * freq / sr;
        double cosw  = std::cos(w0);
        double sinw  = std::sin(w0);
        double alpha = sinw / (2.0 * 0.707);

        double b0 = (1.0 + cosw) / 2.0;
        double b1 = -(1.0 + cosw);
        double b2 = (1.0 + cosw) / 2.0;
        double a0 =  1.0 + alpha;
        double a1 = -2.0 * cosw;
        double a2 =  1.0 - alpha;

        for (int ch = 0; ch < 2; ch++)
            bands[ch].setCoefficients(
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
};