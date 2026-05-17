#ifndef PROCESAR_ESPECTRO_H
#define PROCESAR_ESPECTRO_H

#include "Constantes.h"

namespace esphome {
namespace as7341 {

// Función para calcular la iluminancia (Lux)
// Basado en la curva de sensibilidad fotópica V(lambda)
// E = 683 * sum( S(lambda) * V(lambda) * d_lambda )
inline float calcularIluminancia(const float* espectro) {

    float lux = 0.0f;
    
    for (int i = 0; i < 401; i++){
        lux += espectro[i] * V_LAMBDA[i];
    }

    return 683.0f * lux;
}

// Función para reconstruir el espectro completo (401 puntos)
// Resultado = Kc * Sdut

inline void reconstruirEspectro(const uint32_t* canales, float* espectro_out) {

    for (int i = 0; i < 401; i++) {
        espectro_out[i] = 0;
        for (int j = 0; j < 9; j++) {
            // Leer Kc desde PROGMEM
            float kc_val = KC_MATRIX[i][j];
            espectro_out[i] += kc_val * (float)canales[j];
        }
        // Evitar valores negativos por ruido
        if (espectro_out[i] < 0) espectro_out[i] = 0;
    }
}

} // namespace as7341
} // namespace esphome

#endif
