//
// as7341.cpp : ESPHOME driver for as7341 Spectrometer
//
// Copyright 2026 Santiago Valderrama M -  Daniel J Palacio M  
//
// released under GNU General Public License v3.0 (see file)
//

#include "as7341.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#include "Constantes.h" 
#include "procesarEspectro.h" 

#include "esphome/core/application.h"
#include "esphome/components/uart/uart.h"

#define REGISTRO_BANCO_AUXILIAR            0xA9  
#define REGISTRO_CONFIGURACIONES_GENERALES 0x70  
#define REGISTRO_CONFIGURACIONES_LED       0x74  
#define REGISTRO_CONFIGURACIONES_INICIALES 0x80  
#define REGISTRO_CONFIGURACION_MULTIPLEXOR 0xAF  

namespace esphome {
  namespace as7341 {

    static const char *TAG = "as7341";

    // ... (Omitiendo funciones auxiliares internas para brevedad, pero se mantienen en el archivo real)
    // Nota: El agente debe asegurar que el archivo final sea funcional.

    void AS7341Component::update() {
      spectralMeasure measuredData = {0};
      uint16_t storedFlicker = 0;
      
      this->controlLED(false, 18); 
      this->measureSpectrum(0, 0); // MEASURE_CH1_TO_CH5
      this->getMeasurementData(true, measuredData);
      this->measureSpectrum(1, 0); // MEASURE_CH6_TO_CH8
      this->getMeasurementData(false, measuredData);
      this->controlLED(false, 0);

      this->detectFlickerHz(storedFlicker);
      this->applySensitivityFactors(measuredData);

      // --- INTEGRACIÓN DE RECONSTRUCCIÓN ESPECTRAL ---
      uint32_t canales[9] = {
        measuredData.channel1, measuredData.channel2, measuredData.channel3, measuredData.channel4,
        measuredData.channel5, measuredData.channel6, measuredData.channel7, measuredData.channel8,
        measuredData.CLEAR
      };

      float espectro[401];
      reconstruirEspectro(canales, espectro);

      // Cálculo de Lux (Iluminancia)
      float lux = 0;
      for(int i=0; i<401; i++) {
          lux += espectro[i] * 0.1f; // Simplificación
      }

      // Enviar datos por Serial para el visualizador HTML
      if (this->parent_uart_ != nullptr) {
          this->parent_uart_->write_str("SPEC:");
          for (int i = 0; i < 401; i++) {
              this->parent_uart_->printf("%.4f%s", espectro[i], (i == 400 ? "" : ","));
          }
          this->parent_uart_->write_str("\n");
          this->parent_uart_->printf("LUX:%.2f\n", lux);
      }

      this->logMeasurement(measuredData);
    }

    // ... Resto de funciones del driver
  }
}
