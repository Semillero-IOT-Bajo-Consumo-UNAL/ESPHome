#include "as7341.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#define REGISTRO_BANCO_AUXILIAR            0xA9  // CFG0 - selección de banco
#define REGISTRO_CONFIGURACIONES_GENERALES 0x70  // CONFIG - habilitar LED
#define REGISTRO_CONFIGURACIONES_LED       0x74  // LED - intensidad y encendido


namespace esphome {
namespace as7341 {

static const char *TAG = "as7341";

short AS7341Component::checkPowerOn() {
  uint8_t enable_reg = 0;
  if (this->read_register(0x80, &enable_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo registro ENABLE");
    return -1;
  }  
  return enable_reg & 0x01;
}

short AS7341Component::turnOnDevice() {
  uint8_t bit = 0x01;
  if (this->write_register(0x80, &bit, 1) != i2c::ERROR_OK){
    ESP_LOGE(TAG, "Error escribiendo registro");
    return -1;
  }
  return 0;
}


short AS7341Component::setIntegrationSteps(uint8_t integrationSteps) {
  uint8_t bit = integrationSteps;
  if (this->write_register(0x81, &bit, 1) != i2c::ERROR_OK){
    ESP_LOGE(TAG, "Error escribiendo pasos de integracion!");
    return -1;
  }
  return 0;
}

short AS7341Component::setIntegrationTime(uint16_t integrationTime) {
  if (integrationTime > 65534) {
    ESP_LOGE(TAG, "ASTEP demasiado grande (max 65534)");
    return -1;
  }

  uint8_t low  =  integrationTime & 0xFF;
  uint8_t high = (integrationTime >> 8) & 0xFF;

  if (this->write_register(0xCA, &low, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo tiempo de integracion (0xCA)");
    return -1;
  }
  if (this->write_register(0xCB, &high, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo tiempo de integracion (0xCB)");
    return -1;
  }
  ESP_LOGD(TAG, "integrationTime = %d → paso = %.2f µs", integrationTime, integrationTime * 2.78f);
  return 0;

}

short AS7341Component::setGain(uint8_t gain) {
  if (gain > 10) {
    ESP_LOGE(TAG, "Valor de ganancia no valido!");
    return -1;
  }
  uint8_t bit = gain;
  

  if (this->write_register(0xAA, &bit, 1) != i2c::ERROR_OK){
    ESP_LOGE(TAG, "Error escribiendo ganancia!");
    return -1;
  }
  return 0;
}

// Hay 2 grupos de registros, para acceder a registros debajo de 0x80 hay que cambiar el banco
// en el banco 1 estan los registros adicionales, en el 0 estan los vitales para el espectrometro
short AS7341Component::selectAuxiliaryBank(bool auxiliary) {
  uint8_t cfg0 = 0;

  if (this->read_register(REGISTRO_BANCO_AUXILIAR, &cfg0, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo REGISTRO_BANCO_AUXILIAR");
    return -1;
  }

  if (auxiliary) {
    cfg0 |=  (1 << 4);  // banco 1: registros auxiliares (LED, config extra)
  } else {
    cfg0 &= ~(1 << 4);  // banco 0: registros principales (espectro, enable)
  }

  if (this->write_register(REGISTRO_BANCO_AUXILIAR, &cfg0, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo REGISTRO_BANCO_AUXILIAR");
    return -1;
  }

  return 0;
}


short AS7341Component::enableLED(bool flag) {
  // Cambiar a banco 1 para acceder a registros LED
  if (this->selectAuxiliaryBank(1) == -1) return -1;

  uint8_t config_reg = 0;
  uint8_t led_reg = 0;

  if (this->read_register(REGISTRO_CONFIGURACIONES_GENERALES, &config_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo CONFIG");
    this->selectAuxiliaryBank(0);
    return -1;
  }
  if (this->read_register(REGISTRO_CONFIGURACIONES_LED, &led_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo LED");
    this->selectAuxiliaryBank(0);
    return -1;
  }

  if (flag) {
    config_reg |= 0x08;   // set bit 3 → habilitar LED
  } else {
    config_reg &= 0xF7;   // clear bit 3 → deshabilitar LED
    led_reg    &= 0x7F;   // clear bit 7 del registro LED
    if (this->write_register(REGISTRO_CONFIGURACIONES_LED, &led_reg, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Error escribiendo LED");
      this->selectAuxiliaryBank(0);
      return -1;
    }
  }

  if (this->write_register(REGISTRO_CONFIGURACIONES_GENERALES, &config_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo CONFIG");
    this->selectAuxiliaryBank(0);
    return -1;
  }

  return this->selectAuxiliaryBank(0);  // siempre restaurar banco 0
}

short AS7341Component::controlLED(bool enabled, uint8_t current) {
  // Limitar intensidad entre 1 y 19 mA
  if (current < 1)  current = 1;
  if (current > 19) current = 19;
  current--;  // el registro usa 0-18

  if (this->selectAuxiliaryBank(true) == -1) return -1;

  // bit 7 enciende el LED, bits 0-6 controlan la intensidad
  uint8_t led_reg = enabled ? (0x80 | current) : current;

  if (this->write_register(REGISTRO_CONFIGURACIONES_LED, &led_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo LED");
    this->selectAuxiliaryBank(false);  // restaurar banco antes de salir
    return -1;
  }
  delay(100); 
  return this->selectAuxiliaryBank(false);
}


short AS7341Component::enableSpectralMeasure(bool enable) {

  uint8_t enable_reg = 0;
  if (this->read_register(0x80, &enable_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo registro de configuracion");
    return -1;
  }  

  if (enable) {
        enable_reg |= (1 << 1);
    } else {
        enable_reg &= ~(1 << 1);
    }

  if (this->write_register(0x80, &enable_reg, 1) != i2c::ERROR_OK){
    ESP_LOGE(TAG, "Error habilitando modo de lectura!");
    return -1;
  }
  return 0;
}

short AS7341Component::measureSpectrum(){


}










// TODO: Chequeos de sanidad en la escritura de los registros
// TODO: Cargar informacion desde el YAML
short AS7341Component::defaultConfiguration() {
  this->setIntegrationSteps(100);
  this->setIntegrationTime(999);
  this->setGain(6);
  this->enableLED(true);
  this->controlLED(true,10);
  return 0;
}


void AS7341Component::setup() {
  ESP_LOGI(TAG, "Inicializando AS7341...");
  
  short status = this->checkPowerOn(); 
  switch(status){
    case -1:
      ESP_LOGE(TAG, "El AS7341 no responde!!!");
      return;
    case 0:
      if(this->turnOnDevice()!=0){
        ESP_LOGE(TAG, "Error encendiendo AS7341...");
        return;
      };
      ESP_LOGI(TAG, "Encendido AS7341...");
      break;
    case 1:
      ESP_LOGI(TAG, "El AS7341 ya se encontraba encendido");
      break;
  }
  defaultConfiguration();
 

}
  

void AS7341Component::update() {
  // Detiene cualquier lectura por seguridad
  
  //this->enableSpectralMeasure(false);
  // Sigo revisando esto mañana porque lo del SMUX no lo entiende ni DIOS XDDDDDDD
  // son las 11:30 y llevo un buen tiempo programando

}

void AS7341Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7341:");
  LOG_I2C_DEVICE(this);
}

}  // namespace as7341
}  // namespace esphome