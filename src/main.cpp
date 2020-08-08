#include <INIReader.h>

#include <Constants.h>

#include <defines.h>
#include <xcorr_engine.h>
#include <monitor.h>
#include <file_manager.h>
#include <logger.h>

/*************************
 * PROCESSING PARAMETERS *
 *************************/

 // Low pass filtering
constexpr double cutoff = 15;
constexpr int sampleRate = 1000;

constexpr double RC = 1.0 / (cutoff * 2 * dsperado::PI);
constexpr double dt = 1.0 / sampleRate;
constexpr double alpha = dt / (RC + dt);

// Low pass differentating
constexpr size_t filterLength = 5;
constexpr double coeff = 1.0 / (filterLength * (filterLength + 1));

// XCorr method window
constexpr size_t wSizeAxial = 26;
constexpr size_t wSizeLateral = 4;

int main() {

  logger.init("ust_x.log");

  logger << "UST XCorr " << VERSION << std::endl;
  logger << SEPARATOR;

  UST::FileManager::createDir(OUTPUT_DIR);
  
  // 1) Read config

  INIReader reader(CONFIG);

  if (reader.ParseError() < 0) {
    logger << "Can't load 'config.ini'\n";
    return 1;
  }

  // Data size
  const int beams = reader.GetInteger("data_format", "beams", -1),
            vals = reader.GetInteger("data_format", "vals", -1);

  if (beams == -1 || vals == -1) {
    logger << "Invalid data format!\n";
    return 1;
  }

  // Number of files to skip
  int skip = reader.GetInteger("processing", "skip", 1);

  if (skip == 0) {
    skip = 1;
  }

  // Ultrasound scanning area size in mm
  std::pair<double, double> areaSize = std::make_pair(reader.GetReal("area", "depth", 0),
                                                      reader.GetReal("area", "width", 0));

  const auto dir = reader.Get("processing", "raw_dir", "");
  const auto monitorConfig = reader.Get("processing", "monitoring_config", "");

  // 2) Init logger

  UST::Logger::Instance().setEnabled(true);

  // 3) Allocate arrays for raw data
  short **rawBeamData1 = new short*[beams],
        **rawBeamData2 = new short*[beams],
        **rawBeamDataTmp = new short*[beams];

  double **out = new double*[beams];

  for (int i = 0; i < beams; ++i) {
    rawBeamData1[i] = new short[vals];
    rawBeamData2[i] = new short[vals];
    rawBeamDataTmp[i] = new short[vals];
    out[i] = new double[vals];
  }

  // 4) Allocate vectors for results
  UST::Field tempField(beams);

  for (size_t i = 0; i < beams; ++i) {
    tempField[i].resize(vals);
  }

  UST::PairField areaField;

  double dz, dx;
  if (areaSize.first != 0 && areaSize.second != 0) {
    dx = areaSize.second / (beams - 1);
    dz = areaSize.first / (vals - 1);
  }

  areaField.resize(beams);
  for (int i = 0; i < beams; ++i) {
    areaField[i].resize(vals);
    for (int j = 0; j < vals; ++j) {
      areaField[i][j] = std::make_pair((double)i * dx - areaSize.second / 2,
        (double)j * dz);
    }
  }

  // 5) Init XCorr engine
  
  UST::XCorrEngine engine(wSizeAxial, wSizeLateral, beams, vals);

  // 6) Init Monitor

  Monitoring::Monitor& monitor = Monitoring::Monitor::Instance();
  monitor.init(monitorConfig, beams, vals, areaSize);

  // 7) Init file manager for output

  UST::FileManager fileManager;

  std::vector<std::string> varsToOutput = {"x", "z", "epsilon"};

  fileManager.openBinStream(std::string(OUTPUT_DIR) + "/epsilon.plt", varsToOutput, beams, vals);

  // 8) Start processing

  // For median filter
  double medianWindow[3];
  double min, max;
  int minM, maxM;

  int cnt = 0;
  int step = 1;

  for (const auto& p : std::filesystem::directory_iterator(dir))
  {
    if (p.path().extension() == ".raw") {
      cnt++;

      if (cnt == 1) {
        UST::FileManager::readRAWFile(p.path().string(), rawBeamDataTmp, beams, vals);
        continue;
      } else if (cnt % skip == 0) {
        UST::FileManager::readRAWFile(p.path().string(), rawBeamData2, beams, vals);

        if (cnt % skip == 0) {
          for (int i = 0; i < beams; ++i) {
            for (int j = 0; j < vals; ++j) {
              rawBeamData1[i][j] = rawBeamDataTmp[i][j];
              rawBeamDataTmp[i][j] = rawBeamData2[i][j];
            }
          }
        }
        else {
          break;
        }

        logger << "Step: " << step << ", file number: " << cnt << std::endl;
        step++;

        // 0) Find signal shift
        engine.calcShift(rawBeamData1, rawBeamDataTmp, out);

        // 1) Median filter with a small window (3) to detect outliers
        for (size_t i = 0; i < beams; ++i) {
          for (size_t j = 1; j < vals - 1; ++j) {
            min = 0;
            max = 0;

            medianWindow[0] = out[i][j - 1];
            medianWindow[1] = out[i][j];
            medianWindow[2] = out[i][j + 1];

            min = medianWindow[0];
            minM = 0;
            max = medianWindow[2];
            maxM = 2;

            for (int m = 0; m < 3; ++m) {
              if (medianWindow[m] < min) {
                min = medianWindow[m];
                minM = m;
              }
              else {
                if (medianWindow[m] > max) {
                  max = medianWindow[m];
                  maxM = m;
                }
              }
            }

            out[i][j] = medianWindow[~(minM ^ maxM) & 3];
          }
        }

        // 2) Low pass axial filter
        for (size_t i = 0; i < beams; ++i) {
          for (size_t j = 1; j < vals; ++j) {
            out[i][j] = out[i][j - 1] + (alpha * (out[i][j] - out[i][j - 1]));
          }
        }

        // 3) Low pass lateral filter
        for (size_t j = 0; j < vals; ++j) {
          for (size_t i = 1; i < beams; ++i) {
            out[i][j] = out[i - 1][j] + (alpha * (out[i][j] - out[i - 1][j]));
          }
        }

        // 4) Low pass axial differentiator
        for (size_t i = 0; i < beams; ++i) {
          for (size_t j = filterLength; j < vals - filterLength; ++j) {
            out[i][j] = 0;

            for (int k = 1; k < filterLength + 1; ++k) {
              out[i][j] += coeff * (out[i][j + k] - out[i][j - k]);
            }

            tempField[i][j] += out[i][j];
          }
        }

        // 5) Process monitoring points
        monitor.process(tempField, "epsilon", std::to_string(step));

        // 6) Output results to PLT
        std::vector<std::reference_wrapper<UST::Field>> fieldsToOutput;
        fieldsToOutput.emplace_back(tempField);
        fileManager.writeToBinStream(fieldsToOutput, areaField);
      }
    }
  }

  for (int i = 0; i < beams; ++i) {
    delete rawBeamData1[i];
    delete rawBeamData2[i];
    delete rawBeamDataTmp[i];
    delete out[i];
  }

  delete[] out;
  delete[] rawBeamData1;
  delete[] rawBeamData2;
  delete[] rawBeamDataTmp;

  fileManager.closeBinStream();

  logger << "Done!\n";
  
  return 0;
}
