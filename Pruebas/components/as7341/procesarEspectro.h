#ifndef PROCESAR_ESPECTRO_H
#define PROCESAR_ESPECTRO_H

#include <Arduino.h>
#include "Kc.h"

namespace esphome {
namespace as7341 {

// Función para calcular la iluminancia (Lux)
// Basado en la curva de sensibilidad fotópica V(lambda)
// E = 683 * sum( S(lambda) * V(lambda) * d_lambda )
float calcularIluminancia(const float* espectro) {
    // Curva V(lambda) simplificada o cargada en memoria
    // Por simplicidad, se puede usar una aproximación o la suma ponderada de los canales
    // Una forma común es usar los valores XYZ calculados del espectro
    float lux = 0;
    // ... Implementación del cálculo de Lux
    return lux;
}

// Función para reconstruir el espectro completo (401 puntos)
// Resultado = Kc * Sdut
void reconstruirEspectro(const uint32_t* canales, float* espectro_out) {
    for (int i = 0; i < 401; i++) {
        espectro_out[i] = 0;
        for (int j = 0; j < 9; j++) {
            // Leer Kc desde PROGMEM
            float kc_val = pgm_read_float(&(KC_MATRIX[i][j]));
            espectro_out[i] += kc_val * (float)canales[j];
        }
        // Evitar valores negativos por ruido
        if (espectro_out[i] < 0) espectro_out[i] = 0;
    }
}

} // namespace as7341
} // namespace esphome

#endif
