#include <file_manager.h>

bool UST::FileManager::openBinStream(
  const std::string& fileName, const std::vector<std::string>& vars, int beams, int vals)
{
  IMax = beams;
  JMax = vals;

  III = IMax * JMax * KMax;

  std::string varsStr;

  numVars = vars.size();

  for (int i = 0; i < vars.size(); ++i) {
    if (i != 0) {
      varsStr += " ";
    }
    varsStr += vars[i];
  }

  I = TECINI112(
          (char*)"IJ Ordered Zones",
          (char *)varsStr.c_str(),
          (char *)fileName.c_str(),
          (char*)".",
          &FileFormat,
          &Debug,
          &VIsDouble);

  if (I != 0) {
    logger << ("Error opening output file " + fileName);
    return false;
  }

  b_x = new double[beams * vals];
  b_z = new double[beams * vals];
  b_v = new double*[numVars];

  for (int i = 0; i < vars.size(); ++i) {
    b_v[i] = new double[beams * vals];
  }

  return true;
}

// Write to opened stream
bool UST::FileManager::writeToBinStream(
  const std::vector<std::reference_wrapper<UST::Field>>& fieldsData,
  const UST::PairField& coordData)
{
  double SolTime = pltStreamTime;

  if (fieldsData.empty()) {
    return false;
  }

  I = TECZNE112((char*)"Ordered Zone",
                &ZoneType,
                &IMax,
                &JMax,
                &KMax,
                &ICellMax,
                &JCellMax,
                &KCellMax,
                &SolTime,
                &StrandID,
                &ParentZn,
                &IsBlock,
                &NFConns,
                &FNMode,
                &TotalNumFaceNodes,
                &TotalNumBndryFaces,
                &TotalNumBndryConnections,
                nullptr,
                nullptr,
                nullptr,
                &ShrConn);

  for (int j = 0; j < IMax; ++j) {
    for (int i = 0; i < JMax; ++i) {
      int ind = i * IMax + j;
      b_z[ind] = coordData[j][i].first;
      b_x[ind] = coordData[j][i].second;
      for (int k = 0; k < fieldsData.size(); ++k) {
        b_v[k][ind] = fieldsData[k].get()[j][i];
      }
    }
  }

  I = TECDAT112(&III, b_z, &DIsDouble);
  I = TECDAT112(&III, b_x, &DIsDouble);

  for (int i = 0; i < fieldsData.size(); ++i) {
    I = TECDAT112(&III, b_v[i], &DIsDouble);
  }

  pltStreamTime++;

  return true;
}

// Close stream
void UST::FileManager::closeBinStream() {
  TECEND112();

  delete[] b_x;
  delete[] b_z;

  for (int i = 0; i < numVars; ++i) {
    delete[] b_v[i];
  }

  delete[] b_v;
}

// Read RAW data file
bool UST::FileManager::readRAWFile(const std::string& fileName, short **rawBeamData, int beams, int vals) {
  FILE *in;

#ifdef _MSC_VER
  if (fopen_s(&in, fileName.c_str(), "rb")) {
          logger << "Error opening input file " << fileName << std::endl;
          return false;
        }
#else
  in = fopen(fileName.c_str(), "rb");

  if (!in) {
    logger << "Error opening input file " << fileName << std::endl;
    return false;
  }
#endif

  for (int i = 0; i < beams; ++i) {
    for (int j = 0; j < vals; ++j) {
      fread(&rawBeamData[i][j], sizeof(short), 1, in);
    }
  }

  fclose(in);

  return true;
}