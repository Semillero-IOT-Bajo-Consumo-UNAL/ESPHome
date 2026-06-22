#include "PSpectro.h"
#include "PSpectroData.h"

#include <math.h>

namespace pspectro {
namespace {

float tableRead(const float* table, size_t index) {
  return pgm_read_float(&table[index]);
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

float sampleTable5nm(const float* table,
                     uint16_t startNm,
                     uint16_t stepNm,
                     uint16_t count,
                     uint8_t columns,
                     uint8_t column,
                     float lambdaNm) {
  if (lambdaNm < startNm || lambdaNm > startNm + stepNm * (count - 1)) {
    return 0.0f;
  }

  const float pos = (lambdaNm - startNm) / stepNm;
  const uint16_t i0 = static_cast<uint16_t>(floorf(pos));
  const uint16_t i1 = (i0 + 1 < count) ? i0 + 1 : i0;
  const float t = pos - i0;
  const float a = tableRead(table, static_cast<size_t>(i0) * columns + column);
  const float b = tableRead(table, static_cast<size_t>(i1) * columns + column);
  return lerp(a, b, t);
}

float cieX(float lambdaNm) {
  return sampleTable5nm(data::kCieXyz5nm, data::kCieStartNm, data::kCieStepNm,
                        data::kCieCount, 3, 0, lambdaNm);
}

float cieY(float lambdaNm) {
  return sampleTable5nm(data::kCieXyz5nm, data::kCieStartNm, data::kCieStepNm,
                        data::kCieCount, 3, 1, lambdaNm);
}

float cieZ(float lambdaNm) {
  return sampleTable5nm(data::kCieXyz5nm, data::kCieStartNm, data::kCieStepNm,
                        data::kCieCount, 3, 2, lambdaNm);
}

float vlambda(float lambdaNm) {
  return sampleTable5nm(data::kVlambda5nm, data::kCieStartNm, data::kCieStepNm,
                        data::kCieCount, 1, 0, lambdaNm);
}

float tcs(float lambdaNm, uint8_t sample) {
  return sampleTable5nm(data::kTcs5nm, data::kCieStartNm, data::kCieStepNm,
                        data::kCieCount, data::kTcsCount, sample, lambdaNm);
}

float daylightBasis(float lambdaNm, uint8_t column) {
  return sampleTable5nm(data::kDaylightBasis5nm, data::kDaylightStartNm,
                        data::kDaylightStepNm, data::kDaylightCount, 3, column,
                        lambdaNm);
}

float trapzProduct(const float* lambdaNm,
                   const float* a,
                   size_t n,
                   float (*basis)(float)) {
  if (n < 2) {
    return 0.0f;
  }

  float sum = 0.0f;
  float y0 = a[0] * basis(lambdaNm[0]);
  for (size_t i = 1; i < n; ++i) {
    const float y1 = a[i] * basis(lambdaNm[i]);
    sum += 0.5f * (y0 + y1) * (lambdaNm[i] - lambdaNm[i - 1]);
    y0 = y1;
  }
  return sum;
}

float trapzTriple(const float* lambdaNm,
                  const float* a,
                  const float* b,
                  size_t n,
                  float (*basis)(float)) {
  if (n < 2) {
    return 0.0f;
  }

  float sum = 0.0f;
  float y0 = a[0] * b[0] * basis(lambdaNm[0]);
  for (size_t i = 1; i < n; ++i) {
    const float y1 = a[i] * b[i] * basis(lambdaNm[i]);
    sum += 0.5f * (y0 + y1) * (lambdaNm[i] - lambdaNm[i - 1]);
    y0 = y1;
  }
  return sum;
}

float integrateTcs(const float* lambdaNm,
                   const float* spd,
                   size_t n,
                   uint8_t sample,
                   float (*basis)(float)) {
  if (n < 2) {
    return 0.0f;
  }

  float sum = 0.0f;
  float y0 = spd[0] * tcs(lambdaNm[0], sample) * basis(lambdaNm[0]);
  for (size_t i = 1; i < n; ++i) {
    const float y1 = spd[i] * tcs(lambdaNm[i], sample) * basis(lambdaNm[i]);
    sum += 0.5f * (y0 + y1) * (lambdaNm[i] - lambdaNm[i - 1]);
    y0 = y1;
  }
  return sum;
}

XYZ objectColorXYZ(const float* lambdaNm,
                   const float* spd,
                   size_t n,
                   uint8_t sample,
                   float k) {
  XYZ out;
  out.X = k * integrateTcs(lambdaNm, spd, n, sample, cieX);
  out.Y = k * integrateTcs(lambdaNm, spd, n, sample, cieY);
  out.Z = k * integrateTcs(lambdaNm, spd, n, sample, cieZ);
  return out;
}

}  // namespace

XYZ tristimulus(const float* lambdaNm, const float* spd, size_t n) {
  XYZ out;
  out.X = trapzProduct(lambdaNm, spd, n, cieX);
  out.Y = trapzProduct(lambdaNm, spd, n, cieY);
  out.Z = trapzProduct(lambdaNm, spd, n, cieZ);
  return out;
}

XYZ tristimulusNormalized(const float* lambdaNm, const float* spd, size_t n) {
  XYZ out = tristimulus(lambdaNm, spd, n);
  const float denom = out.Y;
  if (denom > 0.0f) {
    const float k = 100.0f / denom;
    out.X *= k;
    out.Y *= k;
    out.Z *= k;
  }
  return out;
}

float lux(const float* lambdaNm, const float* spd, size_t n) {
  return 683.0f * trapzProduct(lambdaNm, spd, n, vlambda);
}

xy chromaticityXy(const XYZ& value) {
  const float denom = value.X + value.Y + value.Z;
  if (denom <= 0.0f) {
    return {0.0f, 0.0f};
  }
  return {value.X / denom, value.Y / denom};
}

uv xyToUv(const xy& value) {
  const float denom = 12.0f * value.y - 2.0f * value.x + 3.0f;
  if (fabsf(denom) < 1e-12f) {
    return {0.0f, 0.0f};
  }
  return {4.0f * value.x / denom, 6.0f * value.y / denom};
}

cd uvToCd(const uv& value) {
  if (fabsf(value.v) < 1e-12f) {
    return {0.0f, 0.0f};
  }
  return {(4.0f - value.u - 10.0f * value.v) / value.v,
          (1.708f * value.v - 1.481f * value.u + 0.404f) / value.v};
}

float cctMcCamy(const xy& value) {
  const float denom = 0.1858f - value.y;
  if (fabsf(denom) < 1e-12f) {
    return NAN;
  }
  const float n = (value.x - 0.3320f) / denom;
  return 449.0f * n * n * n + 3525.0f * n * n + 6823.3f * n + 5520.33f;
}

void blackbodySpd(float kelvin, const float* lambdaNm, size_t n, float* outSpd) {
  const float c1 = 3.7418e-16f;
  const float c2 = 1.438775225e-2f;
  for (size_t i = 0; i < n; ++i) {
    const float lambdaM = lambdaNm[i] * 1e-9f;
    outSpd[i] = c1 / (powf(lambdaM, 5.0f) * (expf(c2 / (lambdaM * kelvin)) - 1.0f));
  }
}

void daylightSpd(float kelvin, const float* lambdaNm, size_t n, float* outSpd) {
  float xd;
  if (kelvin <= 7000.0f) {
    xd = 0.244063f + 0.09911f * (1e3f / kelvin) +
         2.9678f * (1e6f / (kelvin * kelvin)) -
         4.6070f * (1e9f / (kelvin * kelvin * kelvin));
  } else {
    xd = 0.237040f + 0.24748f * (1e3f / kelvin) +
         1.9018f * (1e6f / (kelvin * kelvin)) -
         2.0064f * (1e9f / (kelvin * kelvin * kelvin));
  }

  const float yd = -3.000f * xd * xd + 2.870f * xd - 0.275f;
  const float M = 0.0241f + 0.2562f * xd - 0.7341f * yd;
  const float M1 = (-1.3515f - 1.7703f * xd + 5.9114f * yd) / M;
  const float M2 = (0.03000f - 31.4424f * xd + 30.0717f * yd) / M;

  for (size_t i = 0; i < n; ++i) {
    outSpd[i] = daylightBasis(lambdaNm[i], 0) +
                M1 * daylightBasis(lambdaNm[i], 1) +
                M2 * daylightBasis(lambdaNm[i], 2);
  }
}

CRIResult cri1995(const float* lambdaNm,
                  const float* testSpd,
                  const float* referenceSpd,
                  size_t n) {
  CRIResult result;
  result.Ra = NAN;
  for (uint8_t i = 0; i < data::kTcsCount; ++i) {
    result.R[i] = NAN;
  }

  const float yTest = trapzProduct(lambdaNm, testSpd, n, cieY);
  const float yRef = trapzProduct(lambdaNm, referenceSpd, n, cieY);
  if (yTest <= 0.0f || yRef <= 0.0f) {
    return result;
  }

  const float kTest = 100.0f / yTest;
  const float kRef = 100.0f / yRef;

  const uv uvTestSource = xyToUv(chromaticityXy(tristimulusNormalized(lambdaNm, testSpd, n)));
  const uv uvRefSource = xyToUv(chromaticityXy(tristimulusNormalized(lambdaNm, referenceSpd, n)));
  const cd cdTestSource = uvToCd(uvTestSource);
  const cd cdRefSource = uvToCd(uvRefSource);

  float raSum = 0.0f;
  for (uint8_t i = 0; i < data::kTcsCount; ++i) {
    const XYZ testXYZ = objectColorXYZ(lambdaNm, testSpd, n, i, kTest);
    const XYZ refXYZ = objectColorXYZ(lambdaNm, referenceSpd, n, i, kRef);

    const uv uvTestSample = xyToUv(chromaticityXy(testXYZ));
    const uv uvRefSample = xyToUv(chromaticityXy(refXYZ));
    const cd cdTestSample = uvToCd(uvTestSample);

    const float cRatio = cdRefSource.c / cdTestSource.c;
    const float dRatio = cdRefSource.d / cdTestSource.d;
    const float denom = 16.518f + 1.481f * cRatio * cdTestSample.c -
                        dRatio * cdTestSample.d;
    const uv adapted = {
        (10.872f + 0.404f * cRatio * cdTestSample.c -
         4.0f * dRatio * cdTestSample.d) /
            denom,
        5.520f / denom};

    const float wTest = 25.0f * cbrtf(testXYZ.Y) - 17.0f;
    const float uTest = 13.0f * wTest * (adapted.u - uvRefSource.u);
    const float vTest = 13.0f * wTest * (adapted.v - uvRefSource.v);

    const float wRef = 25.0f * cbrtf(refXYZ.Y) - 17.0f;
    const float uRef = 13.0f * wRef * (uvRefSample.u - uvRefSource.u);
    const float vRef = 13.0f * wRef * (uvRefSample.v - uvRefSource.v);

    const float deltaE = sqrtf((uTest - uRef) * (uTest - uRef) +
                              (vTest - vRef) * (vTest - vRef) +
                              (wTest - wRef) * (wTest - wRef));
    result.R[i] = 100.0f - 4.6f * deltaE;
    if (i < 8) {
      raSum += result.R[i];
    }
  }

  result.Ra = raSum / 8.0f;
  return result;
}

void reconstructSpectrum(const float* channels,
                         size_t nChannels,
                         const float* reconstructionMatrix,
                         size_t nWavelengths,
                         float* outSpd) {
  for (size_t row = 0; row < nWavelengths; ++row) {
    float value = 0.0f;
    for (size_t col = 0; col < nChannels; ++col) {
      value += reconstructionMatrix[row * nChannels + col] * channels[col];
    }
    outSpd[row] = value;
  }
}

}  // namespace pspectro
