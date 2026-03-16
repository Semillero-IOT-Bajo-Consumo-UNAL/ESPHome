#pragma once
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h" 
namespace esphome {
namespace as7341 {

class AS7341Component : public PollingComponent,
                        public i2c::I2CDevice,
                        public sensor::Sensor { 
 public:
  AS7341Component() = default;
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { 
  return setup_priority::DATA;
  }
 private:
  short checkPowerOn();
  short turnOnDevice();
  short setIntegrationSteps(uint8_t integrationSteps);
  short setIntegrationTime(uint16_t integrationTime); 
  short defaultConfiguration();
  short setGain(uint8_t gain); // del 0 al 10 siendo el valor de la ganancia 2^gain
  short selectAuxiliaryBank(bool enable);
  short enableLED(bool flag);
  short controlLED(bool enabled, uint8_t current);
};

}  // namespace as7341
}  // namespace esphome