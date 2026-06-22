#pragma once

#include <stddef.h>
#include <stdint.h>

namespace pspectro {

struct XYZ {
  float X;
  float Y;
  float Z;
};

struct xy {
  float x;
  float y;
};

struct uv {
  float u;
  float v;
};

struct cd {
  float c;
  float d;
};

struct CRIResult {
  float Ra;
  float R[15];
};

XYZ tristimulus(const float* lambdaNm, const float* spd, size_t n);
XYZ tristimulusNormalized(const float* lambdaNm, const float* spd, size_t n);
float lux(const float* lambdaNm, const float* spd, size_t n);

xy chromaticityXy(const XYZ& value);
uv xyToUv(const xy& value);
cd uvToCd(const uv& value);
float cctMcCamy(const xy& value);

void blackbodySpd(float kelvin, const float* lambdaNm, size_t n, float* outSpd);
void daylightSpd(float kelvin, const float* lambdaNm, size_t n, float* outSpd);

CRIResult cri1995(const float* lambdaNm,
                  const float* testSpd,
                  const float* referenceSpd,
                  size_t n);

void reconstructSpectrum(const float* channels,
                         size_t nChannels,
                         const float* reconstructionMatrix,
                         size_t nWavelengths,
                         float* outSpd);

}  // namespace pspectro
