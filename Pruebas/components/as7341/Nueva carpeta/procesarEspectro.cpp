#include "spectral_processing.h"

void SpectralProcessor::set_calibration(const float matrix[8][8]) {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      calibration_matrix[i][j] = matrix[i][j];
}

SpectralData SpectralProcessor::apply_calibration(const SpectralData &input) {
  SpectralData output{};

  for (int i = 0; i < 8; i++) {
    output.channels[i] = 0;
    for (int j = 0; j < 8; j++) {
      output.channels[i] += calibration_matrix[i][j] * input.channels[j];
    }
  }

  return output;
}