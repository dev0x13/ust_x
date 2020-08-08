#pragma once

#include <defines.h>
#include <thread_pool.h>

namespace UST {
    class XCorrEngine {
    private:
      // Windows for XCorrelation
      UST::Complex ***windows;

      // Outputs for Hilbert transform
      UST::Complex **hField1, **hField2;

      // Data and window sizes
      size_t window_size_lateral, window_size_axial;
      int window_size_by_2_lateral, window_size_by_2_axial;
      size_t size1, size2;

      // Multithreading tasks set up
      UST::Multithreading::ThreadPool tp;
      size_t numThreads = tp.getNumThreads();
      size_t pieceSize = size1 / numThreads;
      size_t div = size1 % numThreads;
      size_t numTasks = numThreads + (div == 0 ? 0 : 1);

      void hilbertTask(
        short **sig1,
        short **sig2,
        size_t begin,
        size_t end);

      void xCorrTask(
        size_t begin,
        size_t end,
        size_t taskId);

      double **out;

    public:
      XCorrEngine(size_t window_size_axial_, size_t window_size_lateral_, size_t size1_, size_t size2_);

      void calcShift(short **sig1, short **sig2, double **out);

      ~XCorrEngine();
    };
}
