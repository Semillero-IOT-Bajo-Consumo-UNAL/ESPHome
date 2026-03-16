#pragma once
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"  // ← agregar este include

namespace esphome {
namespace as7341 {

class AS7341Component : public PollingComponent,
                        public i2c::I2CDevice,
                        public sensor::Sensor {  // ← agregar esta herencia
 public:
  AS7341Component() = default;
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { 
  return setup_priority::DATA;  // 600 — corre después del logger (800)
  }
 private:
  short checkPowerOn();
  short turnOnDevice();
};

}  // namespace as7341
}  // namespace esphome