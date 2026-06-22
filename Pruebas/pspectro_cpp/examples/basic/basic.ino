#include <PSpectro.h>

constexpr size_t N = 95;
float lambdaNm[N];
float testSpd[N];
float refSpd[N];

void setup() {
  Serial.begin(115200);

  for (size_t i = 0; i < N; ++i) {
    lambdaNm[i] = 360.0f + 5.0f * i;
  }

  pspectro::blackbodySpd(3000.0f, lambdaNm, N, testSpd);
  pspectro::daylightSpd(6500.0f, lambdaNm, N, refSpd);

  pspectro::XYZ xyz = pspectro::tristimulusNormalized(lambdaNm, testSpd, N);
  pspectro::xy chroma = pspectro::chromaticityXy(xyz);
  float illuminance = pspectro::lux(lambdaNm, testSpd, N);
  float cct = pspectro::cctMcCamy(chroma);
  pspectro::CRIResult cri = pspectro::cri1995(lambdaNm, testSpd, refSpd, N);

  Serial.printf("x=%.5f y=%.5f lux=%.2f CCT=%.0f Ra=%.2f\n",
                chroma.x, chroma.y, illuminance, cct, cri.Ra);
}

void loop() {}
