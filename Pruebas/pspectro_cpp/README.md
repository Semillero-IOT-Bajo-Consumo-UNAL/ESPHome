# PSpectro C++ Port

Compact C++/Arduino port of the Matlab `pspectro` routines for use on ESP32-C6 projects.
The original Matlab package is copyright Matt Aldrich and distributed under
the BSD-like license included in `LICENSE.txt`.

## What Is Included

- CIE 1931 2-degree tristimulus integration
- `xy`, `uv`, `c,d` conversions
- photopic lux from spectral irradiance
- McCamy CCT approximation
- blackbody SPD
- CIE daylight SPD
- CIE 13.3 / CRI 1995 calculation using the TCS table
- matrix-based AS7341 channel-to-spectrum reconstruction helper

The data tables are sampled every 5 nm to keep flash usage small. The large Matlab `uvbbCCT` lookup table was not ported; `cctMcCamy()` is used instead.

## Basic Use

```cpp
#include <PSpectro.h>

constexpr size_t N = 95;
float lambda[N];
float spd[N];

void setup() {
  for (size_t i = 0; i < N; ++i) {
    lambda[i] = 360.0f + 5.0f * i;
  }

  pspectro::XYZ XYZ = pspectro::tristimulusNormalized(lambda, spd, N);
  pspectro::xy xy = pspectro::chromaticityXy(XYZ);
  float lux = pspectro::lux(lambda, spd, N);
  float cct = pspectro::cctMcCamy(xy);
}
```

## AS7341 Pipeline

The AS7341 does not measure a full spectral power distribution directly. For `pspectro`, first convert the sensor channels into a reconstructed SPD:

```cpp
float channels[10];   // F1..F8 + clear/NIR or your calibrated channel set
float spd[95];        // reconstructed 360:5:830 nm spectrum

// Row-major matrix: 95 rows x 10 columns.
extern const float K[95 * 10];

pspectro::reconstructSpectrum(channels, 10, K, 95, spd);
```

That matrix `K` must come from calibration: measure known reference spectra and solve a regression from AS7341 channels to the desired 5 nm wavelength grid.

## Notes

- Inputs are plain `float*` arrays to avoid heap allocation.
- Spectral integration uses trapezoidal integration, so nonuniform wavelength spacing is accepted.
- For CRI, pass a reference illuminant SPD explicitly. Use `blackbodySpd()` or `daylightSpd()` to generate one when appropriate.
