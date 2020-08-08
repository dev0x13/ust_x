#pragma once

#include <complex>

namespace dsperado {
    namespace XCorr {
        /*
         * Complex 2D cross correlation.
         * Params:
         *   m1 - first 2D array [size1][size2]
         *   m2 - second 2D array [size1][size2]
         *   size1 - ...
         *   size2 - ...
         *   ind1 - ...
         *   ind2 - ...
         *   lag - desired lag (delay)
         * Returns:
         *   Complex cross correlation value in point of interest (ind1, ind2)
         */
        template <typename T>
        dsperado::Complex<T> XCorr2DComplex(
                dsperado::Complex<T> **m1,
                dsperado::Complex<T> **m2,
                const size_t size1, const size_t size2,
                const size_t ind1, const size_t ind2,
                const int lag) {

            dsperado::Complex<T> corr1;

            corr1.r = 0;
            corr1.i = 0;

            size_t bound1, bound2;

            if (lag > 0) {
              bound1 = 0;
              bound2 = lag;
            }
            else {
              bound1 = abs(lag);
              bound2 = 0;
            }

            dsperado::Complex<T> *tmp1, *tmp2;

            for (size_t n_ = 0; n_ < size1 - ind1; ++n_) {
              for (size_t m_ = bound1; m_ < size2 - ind2 - bound2; ++m_) {
                tmp1 = &m1[ind1 + n_][ind2 + m_];
                tmp2 = &m2[ind1 + n_][ind2 + m_ + lag];
                corr1.r += tmp1->r * tmp2->r + tmp1->i * tmp2->i;
                corr1.i += -tmp1->r * tmp2->i + tmp1->i * tmp2->r;
              }
            }

            return corr1;
        }

        /*
         * Normalized complex 2D cross correlation.
         * Params:
         *   m1 - first 2D array [size1][size2]
         *   m2 - second 2D array [size1][size2]
         *   size1 - ...
         *   size2 - ...
         *   ind1 - ...
         *   ind2 - ...
         *   lag - desired lag (delay)
         * Returns:
         *   Complex cross correlation value in point of interest (ind1, ind2)
         */
        template <typename T>
        std::complex<T> XCorr2DNormComplex(
                const std::complex<T> **m1,
                const std::complex<T> **m2,
                const size_t size1, const size_t size2,
                const size_t ind1, const size_t ind2,
                const int lag) {
          
            std::complex<T> corr1(0, 0), corr2(0, 0), corr3(0, 0);

            size_t bound1, bound2;

            if (lag > 0) {
              bound1 = 0;
              bound2 = lag;
            } else {
              bound1 = abs(lag);
              bound2 = 0;
            }

            std::complex<T> *tmp1, *tmp2;

            for (size_t n_ = 0; n_ < size1 - ind1; ++n_) {
                for (size_t m_ = bound1; m_ < size2 - ind2 - bound2; ++m_) {
                    tmp1 = &m1[ind1 + n_][ind2 + m_];
                    tmp2 = &m2[ind1 + n_][ind2 + m_ + lag];
                    corr1 += tmp1 * std::conj(tmp2);
                    corr2 += tmp1 * std::conj(tmp1);
                    corr3 += tmp2 * std::conj(tmp2);
                }
            }

            return corr1 / sqrt(corr2 * corr3);
        }
    }
}
