#include <monitor.h>

#include <Constants.h>

void Monitoring::Monitor::init(const std::string& filename,
  int beams, int vals,
  const UST::DoublePair& areaSize)
{
  UST::FileManager::createDir(std::string(OUTPUT_MONITOR_DIR));

  std::filesystem::path basePath = std::string(OUTPUT_MONITOR_DIR);

  for (auto &t : fields) {
    UST::FileManager::createDir((basePath / t).string());
  }

  _parseConfig(filename, beams, vals, areaSize);

  for (auto &p : monitoringPoints) {
    for (auto &t : fields) {
      outputPointsFiles[t].emplace_back(std::ofstream(basePath / t / (p.name + ".dat")));
    }
  }

  for (auto &l : monitoringLines) {
    for (auto &t : fields) {
      UST::FileManager::createDir((basePath / t / l.name).string());
    }
  }
}

void Monitoring::Monitor::process(const UST::Field& data, const std::string& type, const std::string& label) {
  _processLines(data, type, label);
  _processPoints(data, type, label);
}

Monitoring::Monitor::~Monitor() {
  for (auto &outs : outputPointsFiles) {
    for (auto &out : outs.second) {
      out.close();
    }
  }
}

void Monitoring::Monitor::_processPoints(const UST::Field& data, const std::string& type, const std::string& label) {
  if (outputPointsFiles.count(type) == 0) {
    return;
  }

  for (int i = 0; i < monitoringPoints.size(); ++i) {
    auto mp = monitoringPoints[i];
    if (mp.coord.beam >= data.size() || mp.coord.val >= data[mp.coord.beam].size()) {
      continue;
    }

    outputPointsFiles[type][i] << label << "," << data[mp.coord.beam][mp.coord.val] << std::endl;
  }
}

void Monitoring::Monitor::_processLines(const UST::Field& data, const std::string& type, const std::string& label) {
  std::ofstream out;

  for (auto &l : monitoringLines) {
    std::filesystem::path datFile = std::string(OUTPUT_MONITOR_DIR);
    datFile = datFile / type / l.name / (label + ".dat");

    out.open(datFile);

    if (!out.is_open()) {
      continue;
    }

    if (l.coord.beam == 0) {
      int ind = l.coord.val;

      for (auto &d : data) {
        if (ind >= d.size()) {
          continue;
        } else {
          out << d[ind] << std::endl;
        }
      }
    } else {
      int ind = l.coord.beam;

      if (ind >= data.size()) {
        continue;
      } else {
        for (auto &d : data[ind]) {
          out << d << std::endl;
        }
      }
    }

    out.close();
  }
}

bool Monitoring::Monitor::_parseConfig(const std::string& filename, int beams, int vals, UST::DoublePair areaSize) {
  std::ifstream i(filename);

  if (!i.is_open()) {
    logger << "Invalid input file: " << filename << "\n";
    return false;
  }

  json j;
  i >> j;

  auto monitors = j.at("monitors");

  for (auto &m : monitors) {
    if (m.find("name") == m.end() ||
        ((m.find("x") == m.end() &&
          m.find("z") == m.end()) &&
         (m.find("r") == m.end() ||
          m.find("phi") == m.end()))) {
      logger << "Monitor: skipping invalid monitor\n";
      continue;
    }

    auto name = m["name"].get<std::string>();

    bool isLine = false;

    MonitoringPoint mp;

    mp.coord.beam = 0;
    mp.coord.val = 0;
    mp.name = name;

    if (m.find("r") != m.end() &&
        m.find("phi") != m.end()) {
      double angle = m["phi"].get<double>() * dsperado::PI / 180;
      double r = m["r"].get<double>();
      double x = r * cos(angle), z = r * sin(angle);

      mp.coord.beam = (int) (x / areaSize.second * beams + beams / 2);
      mp.coord.val = (int)(z / areaSize.first * vals + vals / 2);
      isLine = false;
    }
    else {
      if (m.find("x") != m.end()) {
        mp.coord.beam = m["x"].get<int>();
      }
      else {
        isLine = true;
      }

      if (m.find("z") != m.end()) {
        mp.coord.val = m["z"].get<int>();
      }
      else {
        isLine = true;
      }
    }

    if (isLine) {
      if (mp.coord.beam < beams && mp.coord.val < vals) {
        monitoringLines.push_back(mp);
      }
    }
    else {
      if (mp.coord.beam >= 0 && mp.coord.beam < beams &&
          mp.coord.val >= 0 && mp.coord.val < vals) {
        monitoringPoints.push_back(mp);
      }
    }
  }

  return true;
}