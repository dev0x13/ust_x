#include <xcorr_engine.h>

#include <XCorr.h>
#include <HilbertTransformer.h>

#include <algorithm>

// Defected samples in beam number 
static const size_t defects = 14;

UST::XCorrEngine::XCorrEngine(size_t window_size_axial_, size_t window_size_lateral_, size_t size1_, size_t size2_) :
    window_size_axial(window_size_axial_),
    window_size_lateral(window_size_lateral_),
    window_size_by_2_axial(window_size_axial_ / 2),
    window_size_by_2_lateral(window_size_lateral_ / 2),
    size1(size1_),
    size2(size2_)
{

    windows = new Complex**[2 * numTasks];

    for (size_t i = 0; i < 2 * numTasks; ++i) {
        windows[i] = new Complex*[window_size_lateral_];

        for (size_t j = 0; j < window_size_lateral_; ++j) {
            windows[i][j] = new Complex[window_size_axial_];
        }
    }

    hField1 = new Complex*[size1];
    hField2 = new Complex*[size1];

    for (size_t i = 0; i < size1; ++i) {
        hField1[i] = new Complex[size2];
        hField2[i] = new Complex[size2];
    }
}

UST::XCorrEngine::~XCorrEngine() {
    for (size_t i = 0; i < 2 * numTasks; ++i) {
        for (size_t j = 0; j < window_size_lateral; ++j) {
            delete[] windows[i][j];
        }

        delete[] windows[i];
    }

    delete[] windows;

    for (size_t i = 0; i < size1; ++i) {
        delete[] hField1[i];
        delete[] hField2[i];
    }

    delete[] hField1;
    delete[] hField2;
}

void UST::XCorrEngine::hilbertTask(
      short **sig1,
      short **sig2,
      const size_t begin,
      const size_t end)
{
  thread_local static dsperado::HilbertTransformer<double> ht(size2);
  thread_local static UST::Complex *hIn = new Complex[size2];
  size_t i, j;

  double mean1 = 0, mean2 = 0;

  for (i = begin; i < end; ++i) {

    // 1) Fix signal means
    
    for (j = defects; j < size2; ++j) {
      mean1 += sig1[i][j];
      mean2 += sig2[i][j];
    }

    mean1 /= size2 - defects;
    mean2 /= size2 - defects;

    // 2) Perform Hilbert transform
    
    for (j = defects; j < size2; ++j) {
      hIn[j].r = sig1[i][j] - mean1;
      hIn[j].i = 0;
    }

    ht.transform(hIn, hField1[i]);

    for (j = defects; j < size2; ++j) {
      hIn[j].r = sig2[i][j] - mean2;
      hIn[j].i = 0;
    }

    ht.transform(hIn, hField2[i]);
  }
}

void UST::XCorrEngine::xCorrTask(const size_t begin, const size_t end, size_t taskId) {

  // 1) Get windows pointers corresponding to taskId
  
  auto window1 = this->windows[taskId * 2];
  auto window2 = this->windows[taskId * 2 + 1];

  double tmp1, tmp2;

  Complex xCorrRes;

  for (size_t n = begin; n < end; ++n) {
    for (size_t m = defects; m < size2; ++m) {

      size_t wj = 0, wk = 0;

      // 2) Window the hFields
      
      for (long j = (long)n - window_size_by_2_lateral; j < (long)n + window_size_by_2_lateral; ++j) {
        for (long k = (long)m - window_size_by_2_axial; k < (long)m + window_size_by_2_axial; ++k) {
          auto realJ = std::min((long)size1 - 1, std::max(j, (long)0));
          auto realK = std::min((long)size2 - 1, std::max(k, (long)0));

          window1[wj][wk] = hField1[realJ][realK];
          window2[wj][wk] = hField2[realJ][realK];
          wk++;
        }

        wj++;
        wk = 0;
      }

      // 3) Calc XCorrelations with different lags

      xCorrRes = dsperado::XCorr::XCorr2DComplex<double>(
        window1, window2,
        window_size_lateral, window_size_axial,
        window_size_by_2_lateral, window_size_by_2_axial, 0);
      tmp1 = std::atan2(xCorrRes.i, xCorrRes.r);

      xCorrRes = dsperado::XCorr::XCorr2DComplex<double>(
        window1, window2,
        window_size_lateral, window_size_axial,
        window_size_by_2_lateral, window_size_by_2_axial, 1);
      tmp2 = std::atan2(xCorrRes.i, xCorrRes.r);

      xCorrRes = dsperado::XCorr::XCorr2DComplex<double>(
        window1, window2,
        window_size_lateral, window_size_axial,
        window_size_by_2_lateral, window_size_by_2_axial, -1);
      tmp2 -= std::atan2(xCorrRes.i, xCorrRes.r);

      out[n][m] = tmp1 / tmp2;
    }
  }
}

void UST::XCorrEngine::calcShift(
  short **sig1,
  short **sig2,
  double **out)
{
  this->out = out;

  // 1) Perform parallelized Hilbert transform

  this->tp.startTaskBlock(numThreads);

  for (size_t i = 0; i < numThreads; ++i) {
    auto begin = i * pieceSize,
         end = begin + pieceSize;

    this->tp.runTask(&UST::XCorrEngine::hilbertTask, this, sig1, sig2, begin, end);
  }

  if (div != 0) {
    this->hilbertTask(sig1, sig2, size1 - div, size1);
  }

  this->tp.wait();

  // 2) Perform parallelized cross correlation

  this->tp.startTaskBlock(numThreads);

  for (size_t i = 0; i < numThreads; ++i) {
    auto begin = i * pieceSize,
         end = begin + pieceSize;

    this->tp.runTask(&UST::XCorrEngine::xCorrTask, this, begin, end, i);
  }

  if (div != 0) {
    this->xCorrTask(size1 - div, size1, numThreads);
  }

  this->tp.wait();
}
