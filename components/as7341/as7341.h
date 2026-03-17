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
  short measureSpectrum();
  short enableLowPowerMode(bool toggle);
  short enableSpectralMeasure(bool enable);
  short toggleRegisterBit(uint16_t registro, uint16_t bit, bool enable);
  short measureSpectrum(int8_t chToMeasure,int8_t measuringDurationMode);
  short setSMUXCommand(uint8_t smux_cmd);
  short configureSMUX_F1F4_Clear_NIR();
  short configureSMUX_F5F8_Clear_NIR();
  short enableSMUX(bool enable);
  short setGPIOMode(bool input);
  short setMeasurementMode(uint8_t mode);
  short enableSpectralMeasuring(bool enable);
  short isMeasureOver();
};

}  // namespace as7341
}  // namespace esphome