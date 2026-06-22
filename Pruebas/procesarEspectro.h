#ifndef PROCESAR_ESPECTRO_H
#define PROCESAR_ESPECTRO_H

#include "Constantes.h"
#include "cmath"

#include "PSpectro.h"

namespace esphome {
namespace as7341 {

//Resolución de la matriz espectral
//int filas_Kc = 81;

// Función para reconstruir el espectro completo (401 puntos)
// Resultado = Kc * Sdut

inline void reconstruirEspectro(const uint32_t* canales, float* espectro_out) {

    for (int i = 0; i < 401; i++) {
        espectro_out[i] = 0;
        for (int j = 0; j < 9; j++) {
            // Leer Kc desde PROGMEM
            float kc_val = KC_MATRIX[i][j];
            espectro_out[i] += fabs(kc_val * (float)canales[j]);
        }
        // Evitar valores negativos por ruido
        //if (espectro_out[i] < 0) espectro_out[i] = 0;
    }
}

// Función para calcular la iluminancia (Lux)
// Basado en la curva de sensibilidad fotópica V(lambda)
// E = 683 * sum( S(lambda) * V(lambda) * d_lambda )
inline float calcularIluminancia(const float* espectro) {

    float lux = 0.0f;
    
    for (int i = 0; i < 401; i++){
        lux += espectro[i] * V_LAMBDA[i];
    }

    return 683.0f * lux * 1e-3; //La calibración se realizó con un espectrómetro en uW/cm2. Se lleva a W/m2 y de ahí el factor de 0.001.
}

/* Cálculo del M-EDI
A partir del método presentado en la CIE S 026:2018
Integral del producto del espectro medido por la curva de sensibilidad melanópica, 
todo esto dividido por una constante K D65_mel = 1.326
M-EDI = S * Smel / 1.326
*/ 

inline float calcularMEDI(const float* espectro) {

    float medi = 0.0f;

    for (int i = 0; i < 401; i++){
        medi += espectro[i] * Smel[i];
    }
    medi = medi /1.326;
    return medi * 1e-4;
}

} // namespace as7341
} // namespace esphome

#endif
