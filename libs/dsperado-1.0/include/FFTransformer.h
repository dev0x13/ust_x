#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>
#include <memory>
#include <type_traits>
#include <unordered_map>

#include <Complex.h>
#include <Arena.h>
#include <Constants.h>

namespace dsperado {
    /*
     * Fast Fourier Transformer class.
     * Intended to be used if multiple one-size transformations are needed.
     * The first template argument is the desired Complex base type.
     * The second template argument is the FFT direction flag (true - FFT, false - IFFT).
     */
    template <typename T, bool forward = true>
    class FFTransformer {
    private:
        using Type = dsperado::Complex<T>;

        size_t size;
        std::shared_ptr<dsperado::Arena<Type>> arena;
        size_t ind1, ind2;
        Type w, *wn, *tmp0, *tmp1, *tmp2, *tmp3;
        T ang;
        T tmpReal0, tmpReal1;
        Type *complexIn;

        std::unordered_map<size_t, Type> coeffs;

        bool complexInAllocated = false;

        /*
         * These are the helper methods for compile time check if FFT is inversed
         */
        template <bool f>
        inline double calcCoeff(typename std::enable_if<f, size_t>::type n) {
            return -PI2 / n;
        }

        template <bool f>
        inline double calcCoeff(typename std::enable_if<!f, size_t>::type n) {
            return PI2 / n;
        }

        template <bool f>
        inline void divBy2(typename std::enable_if<f, Type*>::type c) {}

        template <bool f>
        inline void divBy2(typename std::enable_if<!f, Type*>::type c) {
            c->r /= 2;
            c->i /= 2;
        }

        void fft(const Type* in, Type* out, size_t n) {
            if (n == 1) {
                return;
            }

            const size_t n_d2 = n / 2;

            auto *s0 = this->arena->allocate(n_d2);
            auto *s1 = this->arena->allocate(n_d2);

            for (ind1 = 0, ind2 = 0; ind1 < n; ind1 += 2, ++ind2) {
                s0[ind2] = in[ind1];
                s1[ind2] = in[ind1 + 1];
            }

            fft(s0, s0, n_d2);
            fft(s1, s1, n_d2);

            w.r = 1;
            w.i = 0;

            wn = &coeffs[n];

            for (ind1 = 0; ind1 < n_d2; ++ind1) {
                tmp0 = &s0[ind1];
                tmp1 = &s1[ind1];
                tmp2 = &out[ind1];
                tmp3 = &out[ind1 + n_d2];

                tmpReal0 = w.r * tmp1->r - w.i * tmp1->i;
                tmpReal1 = w.r * tmp1->i + w.i * tmp1->r;

                tmp2->r = tmp0->r + tmpReal0;
                tmp2->i = tmp0->i + tmpReal1;

                tmp3->r = tmp0->r - tmpReal0;
                tmp3->i = tmp0->i - tmpReal1;

                divBy2<forward>(tmp2);
                divBy2<forward>(tmp3);

                tmpReal0 = w.r;
                w.r = w.r * wn->r - w.i * wn->i;
                w.i = tmpReal0 * wn->i + w.i * wn->r;
            }
        }

    public:
        /*
         * Constructor.
         * Params:
         *   size_ - size of the transformer, restricted to be power of 2
         */
        FFTransformer(size_t size_) : size(size_) {
            assert((size_ & (size_ - 1)) == 0);

            auto sizeLog = (size_t) log2(size_);

            this->arena = std::make_shared<dsperado::Arena<Type>>(size_ * sizeLog);

            coeffs.reserve(sizeLog);

            Type wn;

            for (size_t i = 1; i <= sizeLog; ++i) {
              auto n = pow(2, i);
              auto angle = calcCoeff<forward>(n);
              wn.r = cos(angle);
              wn.i = sin(angle);
              coeffs[n] = wn;
            }
        }

        /*
         * Performs FFT (complex -> complex).
         * Params:
         *   in - pointer to the input Complex<T> number array
         *   out - pointer to the output Complex<T> number array
         */
        void transform(const Type* in, Type* out) {
            this->arena->reset();
            this->fft(in, out, this->size);
        }

        /*
         * Perform FFT (real -> complex).
         * Params:
         *   in - pointer to the input T number array
         *   out - pointer to the output Complex<T> number array
         */
        void transform(const T* in, Type* out) {
            if (!complexInAllocated) {
                complexIn = new Type[size];
                complexInAllocated = true;
            }

            for (ind1 = 0; ind1 < size; ++ind1) {
                complexIn[ind1].r = in[ind1];
                complexIn[ind1].i = 0;
            }

            transform(complexIn, out);
        }

        ~FFTransformer() {
            if (complexInAllocated) {
                delete[] complexIn;
            }
        }
    };
}