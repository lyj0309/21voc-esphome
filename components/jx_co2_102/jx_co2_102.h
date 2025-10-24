#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace jx_co2_102 {

// JX-CO2-102 Infrared CO2 Sensor
// Supports active ASCII reporting mode (default)
// Format: "  xxxx ppm\r\n" sent every 1 second

class JXCO2102Sensor : public uart::UARTDevice, public Component {
 public:
  JXCO2102Sensor() = default;

  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_co2_sensor(sensor::Sensor *co2_sensor) { co2_sensor_ = co2_sensor; }

 protected:
  bool parse_ascii_data_();

  sensor::Sensor *co2_sensor_{nullptr};

  std::vector<uint8_t> rx_buffer_;
};

}  // namespace jx_co2_102
}  // namespace esphome
