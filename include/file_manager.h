#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>

#include <TECIO.h>

#include <defines.h>
#include <logger.h>

namespace UST {
    class FileManager {
    private:
      // For PLT output
      std::ofstream pltStream;
      int pltStreamTime = 0;

      double *b_x;
      double *b_z;
      double **b_v;
      int numVars;

      // For TECIO
      INTEGER4 Debug = 0;
      INTEGER4 VIsDouble = 1;
      INTEGER4 FileFormat = 0;
      INTEGER4 ICellMax = 0;
      INTEGER4 I = 0;
      INTEGER4 JCellMax = 0;
      INTEGER4 KCellMax = 0;
      INTEGER4 DIsDouble = 1;
      INTEGER4 StrandID = 1;
      INTEGER4 ParentZn = 0;
      INTEGER4 IsBlock = 1;
      INTEGER4 NFConns = 0;
      INTEGER4 FNMode = 0;
      INTEGER4 TotalNumFaceNodes = 0;
      INTEGER4 TotalNumBndryFaces = 0;
      INTEGER4 TotalNumBndryConnections = 0;
      INTEGER4 ShrConn = 0;
      INTEGER4 IMax;
      INTEGER4 JMax;
      INTEGER4 KMax = 1;
      INTEGER4 ZoneType = 0;
      INTEGER4 III;
    public:
      // Create directory
      static bool createDir(const std::string& dir) {
        return std::filesystem::create_directory(dir);
      }

      // Open stream for dynamic mode
      bool openBinStream(const std::string& fileName, const std::vector<std::string>& vars, int beams, int vals);

      // Write to opened stream
      bool writeToBinStream(const std::vector<std::reference_wrapper<UST::Field>>& fieldsData, const UST::PairField& coordData);

      // Close stream
      void closeBinStream();

      // Read RAW data file
      static bool readRAWFile(const std::string& fileName, short **rawBeamData, int beams, int vals);
    };
}
