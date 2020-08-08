#pragma once

#include <json.hpp>

#include <defines.h>
#include <file_manager.h>

#include <vector>
#include <fstream>
#include <map>
#include <string>

using json = nlohmann::json;

static std::vector<std::string> fields = { "epsilon" };

// This class provides a way to log field changes at defined coordinates to
// a file
namespace Monitoring {
  struct MonitoringPoint {
    UST::ICoord coord;
    std::string name;

    MonitoringPoint(const std::string& _name, UST::ICoord _coord) : coord(_coord), name(_name) {}
    MonitoringPoint() = default;
  };

  class Monitor {
  public:
    std::vector<MonitoringPoint> monitoringPoints;
    std::vector<MonitoringPoint> monitoringLines;
    std::map<std::string, std::vector<std::ofstream>> outputPointsFiles;

    static Monitor& Instance() {
      static Monitor theSingleInstance;
      return theSingleInstance;
    }

    void init(const std::string& filename,
              int beams, int vals,
              const UST::DoublePair& areaSize);

    void process(const UST::Field& data, const std::string& type, const std::string& label);

    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;

    ~Monitor();

  private:
    void _processPoints(const UST::Field& data, const std::string& type, const std::string& label);

    void _processLines(const UST::Field& data, const std::string& type, const std::string& label);

    bool _parseConfig(const std::string& filename, int beams, int vals, UST::DoublePair areaSize);

    Monitor() = default;
  };
}
