#ifndef PANTALLA_AS7341_H
#define PANTALLA_AS7341_H

#include <stdint.h>

namespace esphome {
namespace as7341 {

struct EstadoPantallaAS7341 {
  uint32_t f1 = 0;
  uint32_t f2 = 0;
  uint32_t f3 = 0;
  uint32_t f4 = 0;
  uint32_t f5 = 0;
  uint32_t f6 = 0;
  uint32_t f7 = 0;
  uint32_t f8 = 0;
  uint32_t clear = 0;
  uint32_t nir = 0;

  float lux = 0.0f;
  float cct = 0.0f;
  float ppfd = 0.0f;
  float dominante_nm = 0.0f;

  uint16_t flicker_hz = 0;
  bool valido = false;
};

inline EstadoPantallaAS7341 pantalla_estado;

inline void actualizarPantallaAS7341(
  uint32_t f1,
  uint32_t f2,
  uint32_t f3,
  uint32_t f4,
  uint32_t f5,
  uint32_t f6,
  uint32_t f7,
  uint32_t f8,
  uint32_t clear,
  uint32_t nir,
  float lux,
  uint16_t flicker_hz
) {
  pantalla_estado.f1 = f1;
  pantalla_estado.f2 = f2;
  pantalla_estado.f3 = f3;
  pantalla_estado.f4 = f4;
  pantalla_estado.f5 = f5;
  pantalla_estado.f6 = f6;
  pantalla_estado.f7 = f7;
  pantalla_estado.f8 = f8;
  pantalla_estado.clear = clear;
  pantalla_estado.nir = nir;
  pantalla_estado.lux = lux;
  pantalla_estado.flicker_hz = flicker_hz;
  pantalla_estado.valido = true;
}

}  // namespace as7341
}  // namespace esphome

#endif