#pragma once

// For INIReader with MSVC
#define _CRT_SECURE_NO_WARNINGS

#define VERSION "1.0"
#define CONFIG "config.ini"
#define OUTPUT_DIR "output"
#define OUTPUT_MONITOR_DIR "output/monitoring"
#define SEPARATOR "-------------------\n"

#include <vector>

#include <Complex.h>

namespace UST {
    typedef std::vector<std::vector<double>> Field;
    typedef std::pair<double, double> DoublePair;
    typedef std::vector<std::vector<DoublePair>> PairField;
    typedef dsperado::Complex<double> Complex;

    struct ICoord {
      int beam = 0;
      int val = 0;
    };

    struct MCoord {
      double x = 0;
      double z = 0;
    };
}
