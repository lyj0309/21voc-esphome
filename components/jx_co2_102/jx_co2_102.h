#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace jx_co2_102 {

// JX-CO2-102 Infrared CO2 Sensor
// Supports active ASCII reporting mode (default)
// Format: "  xxxx ppm\r\n" sent every 1 second
// Also supports manual calibration commands

class JXCO2102Sensor : public PollingComponent, public uart::UARTDevice {
 public:
  JXCO2102Sensor() = default;

  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }
  
  void calibrate_zero();

 protected:
  bool parse_ascii_data_();
  bool jx_co2_write_command_(const uint8_t *command, uint8_t command_len, uint8_t *response, uint8_t response_len);
  uint8_t jx_co2_checksum_(const uint8_t *data, uint8_t len);

  sensor::Sensor *co2_sensor_{nullptr};

  std::vector<uint8_t> rx_buffer_;
};

template<typename... Ts> class JXCO2102CalibrateZeroAction : public Action<Ts...> {
 public:
  JXCO2102CalibrateZeroAction(JXCO2102Sensor *jx_co2_102) : jx_co2_102_(jx_co2_102) {}

  void play(Ts... x) override { this->jx_co2_102_->calibrate_zero(); }

 protected:
  JXCO2102Sensor *jx_co2_102_;
};

}  // namespace jx_co2_102
}  // namespace esphome
