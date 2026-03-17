#include "as7341.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#define REGISTRO_BANCO_AUXILIAR            0xA9  // CFG0 - selección de banco
#define REGISTRO_CONFIGURACIONES_GENERALES 0x70  // CONFIG - habilitar LED
#define REGISTRO_CONFIGURACIONES_LED       0x74  // LED - intensidad y encendido
#define REGISTRO_CONFIGURACIONES_INICIALES 0x80  // CONFIG - Encendido y modos de medicion
#define REGISTRO_CONFIGURACION_MULTIPLEXOR 0xAF  // CONFIG SMUX - Cambia el modo del multiplexor conectado al ADC


namespace esphome {
namespace as7341 {

static const char *TAG = "as7341";




short AS7341Component::checkPowerOn() {
  uint8_t enable_reg = 0;
  if (this->read_register(REGISTRO_CONFIGURACIONES_INICIALES, &enable_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo registro ENABLE");
    return -1;
  }  
  return enable_reg & 0x01;
}

short AS7341Component::turnOnDevice() {
  uint8_t bit = 0x01;
  if (this->write_register(REGISTRO_CONFIGURACIONES_INICIALES, &bit, 1) != i2c::ERROR_OK){
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


short AS7341Component::toggleRegisterBit(uint16_t registro, uint16_t bit, bool enable) {
    uint8_t enable_reg = 0;

    if (this->read_register(registro, &enable_reg, 1) != i2c::ERROR_OK) {
        ESP_LOGE(TAG, "Error leyendo registro de configuracion");
        return -1;
    }

    if (enable) {
        enable_reg |= (1U << bit);
    } else {
        enable_reg &= ~(1U << bit);
    }

    if (this->write_register(registro, &enable_reg, 1) != i2c::ERROR_OK) {
        ESP_LOGE(TAG, "Error habilitando modo de lectura!");
        return -1;
    }

    return 0;
}
short AS7341Component::enableSpectralMeasure(bool enable) {
  this->toggleRegisterBit(0x80,1,enable);
  return 0;
}

short AS7341Component::enableSMUX(bool enable) {
  this->toggleRegisterBit(0x80,4,enable);
  return 0;
}



short AS7341Component::enableLowPowerMode(bool toggle){
  this->toggleRegisterBit(0xA9, 5, toggle);
  return 0;
}

#define GPIO_INPUT_MODE true
#define GPIO_OUTPUT_MODE false

short AS7341Component::setGPIOMode(bool input) {
  uint8_t gpio2 = 0;
  if (this->read_register(0xBE, &gpio2, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo GPIO2");
    return -1;
  }

  if (input) {
    gpio2 |=  (1 << 2);  // set GPIO_IN_EN
    gpio2 &= ~(1 << 1);  // clear GPIO_OUT
  } else {
    gpio2 &= ~(1 << 2);  // clear GPIO_IN_EN
    gpio2 |=  (1 << 1);  // set GPIO_OUT
  }

  if (this->write_register(0xBE, &gpio2, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo GPIO2");
    return -1;
  }
  return 0;
}

#define SMUX_CMD_INITIALIZATION 0
#define SMUX_CMD_READ 1
#define SMUX_CMD_WRITE 2

short AS7341Component::setSMUXCommand(uint8_t smux_cmd) {
  if (smux_cmd > 2) {  // valor 3 es reservado
    ESP_LOGE(TAG, "SMUX_CMD invalido (max 2)");
    return -1;
  }

  uint8_t cfg6 = 0;
  if (this->read_register(REGISTRO_CONFIGURACION_MULTIPLEXOR, &cfg6, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo CFG6");
    return -1;
  }

  cfg6 &= ~(0x03 << 3);          // limpiar bits 4:3
  cfg6 |=  (smux_cmd << 3);      // escribir valor en bits 4:3

  if (this->write_register(REGISTRO_CONFIGURACION_MULTIPLEXOR, &cfg6, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo CFG6");
    return -1;
  }
  return 0;
}


short AS7341Component::configureSMUX_F1F4_Clear_NIR() {
  static const uint8_t register_values[][2] = {
    {0x00, 0x30}, {0x01, 0x01}, {0x02, 0x00}, {0x03, 0x00},
    {0x04, 0x00}, {0x05, 0x42}, {0x06, 0x00}, {0x07, 0x00},
    {0x08, 0x50}, {0x09, 0x00}, {0x0A, 0x00}, {0x0B, 0x00},
    {0x0C, 0x20}, {0x0D, 0x04}, {0x0E, 0x00}, {0x0F, 0x30},
    {0x10, 0x01}, {0x11, 0x50}, {0x12, 0x00}, {0x13, 0x06}
  };

  for (size_t i = 0; i < sizeof(register_values) / sizeof(register_values[0]); i++) {
    uint8_t val = register_values[i][1];
    if (this->write_register(register_values[i][0], &val, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Error escribiendo SMUX F1F4 registro 0x%02X", register_values[i][0]);
      return -1;
    }
  }
  return 0;
}

short AS7341Component::configureSMUX_F5F8_Clear_NIR() {
  static const uint8_t register_values[][2] = {
    {0x00, 0x00}, {0x01, 0x00}, {0x02, 0x00}, {0x03, 0x40},
    {0x04, 0x02}, {0x05, 0x00}, {0x06, 0x10}, {0x07, 0x03},
    {0x08, 0x50}, {0x09, 0x10}, {0x0A, 0x03}, {0x0B, 0x00},
    {0x0C, 0x00}, {0x0D, 0x00}, {0x0E, 0x24}, {0x0F, 0x00},
    {0x10, 0x00}, {0x11, 0x50}, {0x12, 0x00}, {0x13, 0x06}
  };

  for (size_t i = 0; i < sizeof(register_values) / sizeof(register_values[0]); i++) {
    uint8_t val = register_values[i][1];
    if (this->write_register(register_values[i][0], &val, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Error escribiendo SMUX F5F8 registro 0x%02X", register_values[i][0]);
      return -1;
    }
  }
  return 0;
}


short AS7341Component::setMeasurementMode(uint8_t mode) {
  if (mode == 2 || mode > 3) {
    ESP_LOGE(TAG, "Modo de medicion invalido (0=SPM, 1=SYNS, 3=SYND)");
    return -1;
  }

  uint8_t cfg = 0;
  if (this->read_register(0x70, &cfg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo CONFIG");
    return -1;
  }

  cfg &= ~(0x03);       // limpiar bits 1:0
  cfg |=  (mode & 0x03); // escribir modo

  if (this->write_register(0x70, &cfg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error escribiendo CONFIG");
    return -1;
  }
  return 0;
}

short AS7341Component::isMeasureOver() {
  uint8_t status = 0;
  if (this->read_register(0xA3, &status, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo STATUS_2");
    return -1;
  }
  return (status & (1 << 6)) != 0;
}

#define MEASURE_CH1_TO_CH5 0
#define MEASURE_CH6_TO_CH8 1

#define MEASURE_DURATION_DEFAULT 0 // La medicion dura lo configurado por el usuario
#define MEASURE_DURATION_EXTERNAL_GPIO_PULSE 1 // La medicion inicia con un pulso por GPIO y dura lo configurado
#define MEASURE_DURATION_EXTERNAL_GPIO_HIGH 3 // La medicion esta "prendida" mientras el GPIO este en 5V

short AS7341Component::measureSpectrum(int8_t chToMeasure,int8_t measuringDurationMode){
  this->enableLowPowerMode(false);
  this->enableSpectralMeasure(false);
  this->setSMUXCommand(SMUX_CMD_WRITE);

  // Configuramos el SMUX para leer los canales que requerimos
  // TODO: Investigar bien esto y pasarlo a una funcion
  switch (chToMeasure)
  {
  case MEASURE_CH1_TO_CH5:
    this->configureSMUX_F1F4_Clear_NIR();
    break;
  case MEASURE_CH6_TO_CH8:
    this->configureSMUX_F5F8_Clear_NIR();
    break;
  default:
    ESP_LOGE(TAG, "Seleccione un rango de canales valido.");
    return -1;
    break;
  }

  if(measuringDurationMode==MEASURE_DURATION_EXTERNAL_GPIO_HIGH){
    ESP_LOGE(TAG, "Este modo no ha sido implementado todavia.");
    return -1;
  }

  setMeasurementMode(measuringDurationMode);
  if(measuringDurationMode==MEASURE_DURATION_EXTERNAL_GPIO_PULSE){
    this->setGPIOMode(GPIO_INPUT_MODE);
  }

  this->enableSMUX(true);
  this->enableSpectralMeasure(true);

  if(measuringDurationMode==MEASURE_DURATION_EXTERNAL_GPIO_PULSE){
    while(!this->isMeasureOver()){
      delay(1);
    }
  }
  ESP_LOGI(TAG, "Se ha finalizado una lectura espectral: %d/2",(chToMeasure+1));
  return 0;
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
  this->measureSpectrum(MEASURE_CH1_TO_CH5,MEASURE_DURATION_DEFAULT);

  this->measureSpectrum(MEASURE_CH6_TO_CH8,MEASURE_DURATION_DEFAULT);

}

void AS7341Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7341:");
  LOG_I2C_DEVICE(this);
}

}  // namespace as7341
}  // namespace esphome