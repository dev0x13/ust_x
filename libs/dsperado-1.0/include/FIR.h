#pragma once

#include <vector>
#include <cassert>
#include <cstddef>
#include <map>

#include <Constants.h>

namespace dsperado {
  namespace FIR {
    /*
     * Low pass filter.
     * Params:
     *   inOut: pointer to the input-output array
     *   inOutSize: input-output array size
     *   cutoff: filter cutoff frequency
     *   sampleRate: signal sample rate
     */
    void lowPass(double *inOut, size_t inOutSize, int cutoff, int sampleRate) {
        double RC = oneDivPI2 / cutoff;
        double dt = 1.0 / sampleRate;
        double alpha = dt / (RC + dt);

        for (size_t i = 1; i < inOutSize; ++i) {
            inOut[i] = inOut[i - 1] + (alpha * (inOut[i] - inOut[i - 1]));
        }
    }

    /*
     * High pass filter.
     * Params:
     *   inOut: pointer to the input-output array
     *   inOutSize: input-output array size
     *   cutoff: filter cutoff frequency
     *   sampleRate: signal sample rate
     */
    void highPass(double *inOut, size_t inOutSize, int cutoff, int sampleRate) {
        double RC = oneDivPI2 / cutoff;
        double dt = 1.0 / sampleRate;
        double alpha = RC / (RC + dt);

        double prev = inOut[0], tmp;

        for (size_t i = 1; i < inOutSize; ++i) {
            tmp = inOut[i];
            inOut[i] = alpha * (inOut[i - 1] + inOut[i] - prev);
            prev = tmp;
        }
    }

    /*
     * Smoothed digital differentiator I.
     * Coefficients are cached once designed.
     * Params:
     *   inOut: reference to the input-output vector
     *   inOutSize: input-output array size
     *   coeffsNum: number of coefficients (filter length = 2 * coeffsNum + 1)
     */
    void smoothedDD1(double *inOut, size_t inOutSize, size_t coeffsNum) {
        assert(coeffsNum >= 2);

        static std::map<int, std::vector<double>> coeffsMap;

        auto& coeffs = coeffsMap[coeffsNum];

        if (coeffs.empty()) {
            for (size_t i = 0; i < coeffsNum; ++i) {
                coeffs.emplace_back(1.0 / coeffsNum / (coeffsNum + 1));
            }
        }

        for (size_t i = 0; i < inOutSize; ++i) {
            for (size_t j = 1; j < coeffsNum + 1; ++j) {
                inOut[i] += coeffs[j - 1] * (inOut[std::min(i + j, inOutSize - 1)] - inOut[std::max(i - j, (size_t) 0)]);
            }
        }
    }
  }
}
