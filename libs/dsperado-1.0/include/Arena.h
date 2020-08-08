#pragma once

#include <cassert>
#include <cstddef>

namespace dsperado {
    /*
     * Helper memory arena class. Not thread-safe.
     */
    template <typename T>
    class Arena {
    private:
        T* arena;
        size_t offset = 0;
        size_t size;
    public:
        Arena(size_t size_): size(size_) {
            this->arena = new T[size_];
        }

        ~Arena() {
            delete[] arena;
        }

        T* allocate(size_t length) {
          auto newOffset = this->offset + length;
          assert(newOffset <= this->size);

          auto p = this->arena + this->offset;

          offset += length;

          return p;
        }

        void reset() {
            offset = 0;
        }
    };
}