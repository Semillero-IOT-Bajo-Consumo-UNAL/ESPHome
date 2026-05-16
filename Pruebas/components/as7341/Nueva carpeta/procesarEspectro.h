#pragma once
#include <vector>

struct SpectralData {
  float channels[8];
};

class SpectralProcessor {
 public:
  void set_calibration(const float matrix[8][8]);
  SpectralData apply_calibration(const SpectralData &input);

 private:
  float calibration_matrix[8][8];
};