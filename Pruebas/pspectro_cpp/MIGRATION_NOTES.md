# Migration Notes

## Matlab To C++ Mapping

| Matlab function | C++ replacement |
| --- | --- |
| `gettristimulus2deg` | `pspectro::tristimulus()` |
| `gettristimulus2degn` | `pspectro::tristimulusNormalized()` |
| `getlux` | `pspectro::lux()` |
| `getxyz` | `pspectro::chromaticityXy()` |
| `xytouv` | `pspectro::xyToUv()` |
| `uvtocd` | `pspectro::uvToCd()` |
| `getbbradiator` | `pspectro::blackbodySpd()` |
| `getdilluminantspd` | `pspectro::daylightSpd()` |
| `getcri1995` | `pspectro::cri1995()` |
| `getuvbbCCT` | `pspectro::cctMcCamy()` approximation |

## ESP32-C6 / AS7341 Decision

The AS7341 provides discrete spectral channels, not a full 1 nm or 5 nm SPD.
For colorimetric functions you need one of these front ends:

1. Direct calibration from AS7341 channels to `XYZ`.
2. Reconstruction from AS7341 channels to a 5 nm SPD, then use this library.

This port supports option 2 with:

```cpp
pspectro::reconstructSpectrum(channels, nChannels, K, nWavelengths, spd);
```

`K` is your calibrated row-major reconstruction matrix. For example, with 10
AS7341 channels and a 360:5:830 nm SPD:

```cpp
float K[95 * 10];
```

## Data Tables

The original Matlab data tables were interpolated to 5 nm:

- CIE 1931 2-degree CMFs: 360 to 830 nm, 95 rows
- V(lambda): 360 to 830 nm, 95 rows
- CIE TCS samples: 360 to 830 nm, 95 rows
- CIE daylight basis: 300 to 830 nm, 107 rows

The large `uvbbCCT` lookup table was not ported because it is too large for a
small embedded library. `cctMcCamy()` is compact and useful for first estimates,
but it is less precise than a Planckian locus nearest-distance calculation.
