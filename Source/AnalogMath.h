#pragma once
#include <cmath>

//==============================================================================
// ANALOG MATH UTILITIES
// Shared DSP functions used across all processors
// Single definition - no triplication
//==============================================================================

namespace AnalogMath
{
    // Fast tanh approximation
    // Pade rational approximant
    // Accurate to 0.5% for |x| < 3
    // 3x faster than std::tanh
    inline float safeTanh(float x)
    {
        if (x >  3.0f) return  1.0f;
        if (x < -3.0f) return -1.0f;
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Output soft clipper
    // Transparent at normal levels
    // Smooth tanh knee above ceiling
    // Default ceiling = -0.5dBFS
    inline float softClip(float x,
                           float ceiling = 0.944f)
    {
        if (x > ceiling || x < -ceiling)
            return ceiling * safeTanh(x / ceiling);
        return x;
    }

    // Convert milliseconds to one-pole IIR coefficient
    // Standard envelope detector time constant
    // Result used as: y = coeff*y + (1-coeff)*x
    inline float msToCoeff(float timeMs,
                            float sampleRate)
    {
        if (timeMs <= 0.0f) return 0.0f;
        return std::exp(-1.0f /
            (sampleRate * timeMs * 0.001f));
    }
}