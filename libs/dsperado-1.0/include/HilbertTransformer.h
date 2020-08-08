#pragma once

#include <vector>
#include <complex>
#include <cassert>
#include <FFTransformer.h>

namespace dsperado {
    /*
     * Hilbert transformer class.
     * Actually creates complex analytic signal, the imaginary part of which
     *   is the actual Hilbert transform result.
     * Intended to be used if multiple one-size transformations are needed.
     * The first template argument is the desired Complex base type.
     */
    template<typename T>
    class HilbertTransformer {
    private:
        using Type = dsperado::Complex<T>;

        size_t bufferSize;
        size_t limit1, limit2;
        size_t ind;
        Type *buffer, *complexIn;
        std::shared_ptr<dsperado::FFTransformer<T, true>> FFT;
        std::shared_ptr<dsperado::FFTransformer<T, false>> IFFT;

        bool complexInAllocated = false;

    public:
        /*
         * Constructor.
         * Params:
         *   bufferSize_ - size of the transformer
         */
        HilbertTransformer(size_t bufferSize_) : bufferSize(bufferSize_) {
            assert((bufferSize_ & (bufferSize_ - 1)) == 0);

            FFT = std::make_shared<dsperado::FFTransformer<T, true>>(bufferSize_);
            IFFT = std::make_shared<dsperado::FFTransformer<T, false>>(bufferSize_);

            buffer = new dsperado::Complex<T>[bufferSize_];

            auto mod_ = this->bufferSize % 2;
            limit1 = (this->bufferSize + mod_) / 2;
            limit2 = limit1 + ~mod_;
        }

        /*
         * Performs Hilbert transform (complex -> complex).
         * Params:
         *   in - pointer to the input Complex<T> number array
         *   out - pointer to the output Complex<T> number array
         */
        void transform(const Type *in, Type *out) {

            FFT->transform(in, buffer);

            for (ind = 1; ind < limit1; ++ind) {
                buffer[ind].r *= 2;
                buffer[ind].i *= 2;
            }

            for (ind = limit2; ind < bufferSize; ++ind) {
                buffer[ind].r = 0;
                buffer[ind].i = 0;
            }

            IFFT->transform(buffer, out);
        }

        /*
         * Performs Hilbert transform (real -> complex).
         * Params:
         *   in - pointer to the input T number array
         *   out - pointer to the output Complex<T> number array
         */
        void transform(const T *in, Type *out) {
            if (!complexInAllocated) {
                complexIn = new Type[bufferSize];
                complexInAllocated = true;
            }

            for (ind = 0; ind < bufferSize; ++ind) {
                complexIn[ind].r = in[ind];
                complexIn[ind].i = 0;
            }

            transform(complexIn, out);
        }

        ~HilbertTransformer() {
            delete[] buffer;

            if (complexInAllocated) {
                delete[] complexIn;
            }
        }
    };
}
