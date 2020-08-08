#pragma once

#include <string>
#include <fstream>
#include <iostream>

namespace UST {
  // Simple logger class
  class Logger {
  public:
    static Logger& Instance() {
      static Logger s;
      return s;
    }

    void init(const std::string& logName) {
      if (!logFile.is_open()) {
        logFile.open(logName);

        if (logFile.is_open()) {
          enabledFile = true;
        }
      }
    }

    void setEnabled(bool enabled) {
      this->enabled = enabled;
    }

    Logger &operator<<(std::ostream& (*pf) (std::ostream&)) {
      if (enabled) {
        std::cout << pf;
        if (enabledFile) {
          logFile << pf;
        }
      }
      logFile.flush();
      return *this;
    }

    template <typename T>
    Logger& operator<<(const T& info) {
      if (enabled) {
        std::cout << info;
        if (enabledFile) {
          logFile << info;
        }
      }
      logFile.flush();
      return *this;
    }

    Logger(Logger const&) = delete;
    Logger& operator= (Logger const&) = delete;
  private:
    Logger() = default;

    ~Logger() {
      logFile.close();
    }

    std::ofstream logFile;
    bool enabled = true;
    bool enabledFile = true;
  };
}

static UST::Logger& logger = UST::Logger::Instance();