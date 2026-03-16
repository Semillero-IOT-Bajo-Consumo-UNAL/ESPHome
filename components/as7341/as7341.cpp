#include "as7341.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7341 {

static const char *TAG = "as7341";

short AS7341Component::checkPowerOn() {
  uint8_t enable_reg = 0;
  if (this->read_register(0x80, &enable_reg, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Error leyendo registro ENABLE");
    return -1;
  }  
  return enable_reg;
}

short AS7341Component::turnOnDevice() {
  uint8_t bit = 0x01;
if (this->write_register(0x80, &bit, 1) != i2c::ERROR_OK){
    ESP_LOGE(TAG, "Error escribiendo registro");
    return -1;
  }
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
      this->turnOnDevice();
      ESP_LOGI(TAG, "Encendiendo AS7341...");

      break;
    case 1:
      ESP_LOGI(TAG, "El AS7341 ya se encontraba encendido");
      return;
    default:
      ESP_LOGI(TAG, "WTFFFF");
      return;
  }
  

}
  

void AS7341Component::update() {

}

void AS7341Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7341:");
  LOG_I2C_DEVICE(this);
}

}  // namespace as7341
}  // namespace esphome