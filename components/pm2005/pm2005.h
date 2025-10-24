#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace pm2005 {

// PM2005 Laser Particle Sensor
// Supports UART-TTL communication (9600 baud, 8N1)
// Protocol: Custom binary protocol for particle measurement

// Protocol constants
static const uint8_t PM2005_CMD_HEADER = 0x11;
static const uint8_t PM2005_RESP_HEADER = 0x16;

// Commands
static const uint8_t PM2005_CMD_OPEN_CLOSE = 0x0C;
static const uint8_t PM2005_CMD_READ_PARTICLE = 0x0B;
static const uint8_t PM2005_CMD_READ_MASS = 0x0B;

// Response lengths
static const uint8_t PM2005_RESP_HEADER_LEN = 3;  // HEADER + LEN + CMD
static const uint8_t PM2005_PARTICLE_DATA_LEN = 18;  // Full response length

// Measurement states
enum PM2005State {
  PM2005_STATE_IDLE = 0,
  PM2005_STATE_WAIT_RESPONSE = 1,
  PM2005_STATE_MEASURING = 2,
};

class PM2005Sensor : public uart::UARTDevice, public Component {
 public:
  PM2005Sensor() = default;

  void setup() override;
  void dump_config() override;
  void loop() override;
  void update();
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_pm_0_5_sensor(sensor::Sensor *pm_0_5_sensor) { pm_0_5_sensor_ = pm_0_5_sensor; }
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { pm_2_5_sensor_ = pm_2_5_sensor; }
  void set_pm_10_0_sensor(sensor::Sensor *pm_10_0_sensor) { pm_10_0_sensor_ = pm_10_0_sensor; }
  void set_pm_2_5_mass_sensor(sensor::Sensor *pm_2_5_mass_sensor) { pm_2_5_mass_sensor_ = pm_2_5_mass_sensor; }
  void set_pm_10_0_mass_sensor(sensor::Sensor *pm_10_0_mass_sensor) { pm_10_0_mass_sensor_ = pm_10_0_mass_sensor; }

 protected:
  void send_command_(uint8_t cmd, const uint8_t *data, uint8_t data_len);
  void open_measurement_();
  void read_particle_data_();
  void read_mass_data_();
  bool parse_response_();
  uint8_t calculate_checksum_(const uint8_t *data, uint8_t len);

  sensor::Sensor *pm_0_5_sensor_{nullptr};
  sensor::Sensor *pm_2_5_sensor_{nullptr};
  sensor::Sensor *pm_10_0_sensor_{nullptr};
  sensor::Sensor *pm_2_5_mass_sensor_{nullptr};
  sensor::Sensor *pm_10_0_mass_sensor_{nullptr};

  std::vector<uint8_t> rx_buffer_;
  PM2005State state_{PM2005_STATE_IDLE};
  uint32_t last_measurement_time_{0};
  uint32_t last_command_time_{0};
  bool measuring_{false};
  bool read_mass_next_{false};
};

}  // namespace pm2005
}  // namespace esphome
