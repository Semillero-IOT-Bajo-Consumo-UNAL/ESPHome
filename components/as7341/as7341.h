//
// as7341.h : ESPHOME driver for as7341 Spectrometer
//
// Copyright 2026 Santiago Valderrama M -  Daniel J Palacio M  
//
// released under GNU General Public License v3.0 (see file)
//



#pragma once
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h" 


typedef struct{
    uint32_t channel1;/**<channel1 diode data>*/
    uint32_t channel2;/**<channel2 diode data>*/
    uint32_t channel3;/**<channel3 diode data>*/
    uint32_t channel4;/**<channel4 diode data>*/
    uint32_t channel5;/**<channel5 diode data>*/
    uint32_t channel6;/**<channel6 diode data>*/
    uint32_t channel7;/**<channel7 diode data>*/
    uint32_t channel8;/**<channel8 diode data>*/
    uint32_t CLEAR;/**<clear diode data>*/
    uint32_t NIR;/**<NIR diode data>*/
  }spectralMeasure;


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
  void set_gain(uint8_t gain)                  { gain_ = gain; }
  void set_integration_steps(uint8_t steps)    { integration_steps_ = steps; }
  void set_integration_time(uint16_t time)     { integration_time_ = time; }
  void set_led_enabled(bool enabled)           { led_enabled_ = enabled; }
  void set_led_current(uint8_t current)        { led_current_ = current; }
  void set_sensitivity_factor(uint8_t idx, float val) {
    if (idx < 8) sensitivity_factors_[idx] = val;
  }
  float sensitivity_factors_[8] = {24.5f, 12.3f, 6.4f, 3.5f, 2.3f, 1.6f, 1.0f, 1.3f};
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
  short getChannelData(uint8_t channel, uint32_t &data);
  short getMeasurementData(bool firstHalf, spectralMeasure &datos);
  void logMeasurement(const spectralMeasure &datos);
  short configureSMUX_Flicker();
  short detectFlickerHz(uint16_t &result);
  void applySensitivityFactors(spectralMeasure &datos);

  uint8_t  gain_              = 6;
  uint8_t  integration_steps_ = 100;
  uint16_t integration_time_  = 999;
  bool     led_enabled_       = true;
  uint8_t  led_current_       = 10;

};

}  // namespace as7341
}  // namespace esphome