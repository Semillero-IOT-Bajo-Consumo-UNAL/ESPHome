#include <PSpectro.h>

constexpr size_t kChannels = 10;
constexpr size_t kSpectrum = 95;

float lambdaNm[kSpectrum];
float channels[kChannels];
float spd[kSpectrum];

// Replace with your calibrated row-major matrix: 95 x 10.
const float K[kSpectrum * kChannels] = {0.0f};

void readAs7341Channels(float* outChannels) {
  // Fill with F1..F8, clear and NIR, or with the channel set used in calibration.
  for (size_t i = 0; i < kChannels; ++i) {
    outChannels[i] = 0.0f;
  }
}

void setup() {
  Serial.begin(115200);
  for (size_t i = 0; i < kSpectrum; ++i) {
    lambdaNm[i] = 360.0f + 5.0f * i;
  }
}

void loop() {
  readAs7341Channels(channels);
  pspectro::reconstructSpectrum(channels, kChannels, K, kSpectrum, spd);

  pspectro::XYZ xyz = pspectro::tristimulusNormalized(lambdaNm, spd, kSpectrum);
  pspectro::xy chroma = pspectro::chromaticityXy(xyz);
  float illuminance = pspectro::lux(lambdaNm, spd, kSpectrum);

  Serial.printf("x=%.5f y=%.5f lux=%.2f\n", chroma.x, chroma.y, illuminance);
  delay(1000);
}
