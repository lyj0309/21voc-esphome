#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace two_one_voc {

static const uint8_t PACKET_SIZE = 12;
static const uint8_t HEADER_BYTE = 0x2C;

class FiveInOneSensor : public uart::UARTDevice, public Component {
 public:
  FiveInOneSensor() = default;

  void setup() override;
  void dump_config() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_voc_sensor(sensor::Sensor *voc_sensor) { voc_sensor_ = voc_sensor; }
  void set_formaldehyde_sensor(sensor::Sensor *formaldehyde_sensor) { 
    formaldehyde_sensor_ = formaldehyde_sensor; 
  }
  void set_eco2_sensor(sensor::Sensor *eco2_sensor) { eco2_sensor_ = eco2_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { 
    temperature_sensor_ = temperature_sensor; 
  }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { 
    humidity_sensor_ = humidity_sensor; 
  }

 protected:
  bool parse_data_();
  bool validate_checksum_(const uint8_t *data);
  int16_t parse_temperature_(uint16_t raw_value);

  sensor::Sensor *voc_sensor_{nullptr};
  sensor::Sensor *formaldehyde_sensor_{nullptr};
  sensor::Sensor *eco2_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};

  std::vector<uint8_t> rx_buffer_;
};

}  // namespace two_one_voc
}  // namespace esphome
