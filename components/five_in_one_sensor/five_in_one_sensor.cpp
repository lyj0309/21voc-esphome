#include "five_in_one_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace five_in_one_sensor {

static const char *const TAG = "five_in_one_sensor";

void FiveInOneSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Five-in-One Sensor...");
}

void FiveInOneSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Five-in-One Sensor:");
  LOG_SENSOR("  ", "VOC", this->voc_sensor_);
  LOG_SENSOR("  ", "Formaldehyde", this->formaldehyde_sensor_);
  LOG_SENSOR("  ", "eCO2", this->eco2_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
  this->check_uart_settings(9600);
}

void FiveInOneSensor::loop() {
  // Read available data from UART
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // If we find the header byte, start a new packet
    if (byte == HEADER_BYTE && this->rx_buffer_.empty()) {
      this->rx_buffer_.push_back(byte);
    } else if (!this->rx_buffer_.empty()) {
      this->rx_buffer_.push_back(byte);
      
      // If we have a complete packet, parse it
      if (this->rx_buffer_.size() == PACKET_SIZE) {
        if (this->parse_data_()) {
          ESP_LOGV(TAG, "Successfully parsed data packet");
        } else {
          ESP_LOGW(TAG, "Invalid data packet received");
        }
        this->rx_buffer_.clear();
      }
    }
  }
  
  // Clear buffer if it's too large (shouldn't happen with valid data)
  if (this->rx_buffer_.size() > PACKET_SIZE) {
    ESP_LOGW(TAG, "Buffer overflow, clearing");
    this->rx_buffer_.clear();
  }
}

bool FiveInOneSensor::validate_checksum_(const uint8_t *data) {
  // Checksum = sum of first 11 bytes inverted + 1
  uint8_t sum = 0;
  for (int i = 0; i < 11; i++) {
    sum += data[i];
  }
  uint8_t expected_checksum = (~sum) + 1;
  
  if (data[11] != expected_checksum) {
    ESP_LOGW(TAG, "Checksum failed: expected 0x%02X, got 0x%02X", expected_checksum, data[11]);
    return false;
  }
  return true;
}

int16_t FiveInOneSensor::parse_temperature_(uint16_t raw_value) {
  // Check if temperature is negative (MSB is 1)
  if (raw_value & 0x8000) {
    // Negative temperature: -(0xFFFF - raw_value)
    // Example: -10°C = 0xFFF5, 0xFFFF - 0xFFF5 = 0x000A = 10, then negate = -10
    return -static_cast<int16_t>(0xFFFF - raw_value);
  }
  return static_cast<int16_t>(raw_value);
}

bool FiveInOneSensor::parse_data_() {
  if (this->rx_buffer_.size() != PACKET_SIZE) {
    return false;
  }
  
  const uint8_t *data = this->rx_buffer_.data();
  
  // Verify header
  if (data[0] != HEADER_BYTE) {
    ESP_LOGW(TAG, "Invalid header: 0x%02X", data[0]);
    return false;
  }
  
  // Verify checksum
  if (!this->validate_checksum_(data)) {
    return false;
  }
  
  // Parse VOC (bytes 1-2): Data[1]*256 + Data[2]
  uint16_t voc = (data[1] << 8) | data[2];
  if (this->voc_sensor_ != nullptr) {
    this->voc_sensor_->publish_state(voc);
  }
  ESP_LOGD(TAG, "VOC: %d µg/m³", voc);
  
  // Parse Formaldehyde (bytes 3-4): Data[3]*256 + Data[4]
  uint16_t formaldehyde = (data[3] << 8) | data[4];
  if (this->formaldehyde_sensor_ != nullptr) {
    this->formaldehyde_sensor_->publish_state(formaldehyde);
  }
  ESP_LOGD(TAG, "Formaldehyde: %d µg/m³", formaldehyde);
  
  // Parse eCO2 (bytes 5-6): Data[5]*256 + Data[6]
  uint16_t eco2 = (data[5] << 8) | data[6];
  if (this->eco2_sensor_ != nullptr) {
    this->eco2_sensor_->publish_state(eco2);
  }
  ESP_LOGD(TAG, "eCO2: %d PPM", eco2);
  
  // Parse Temperature (bytes 7-8): Data[7]*256 + Data[8]
  // Unit is 0.1°C, with special handling for negative values
  uint16_t temp_raw = (data[7] << 8) | data[8];
  int16_t temp_value = this->parse_temperature_(temp_raw);
  float temperature = temp_value * 0.1f;
  if (this->temperature_sensor_ != nullptr) {
    this->temperature_sensor_->publish_state(temperature);
  }
  ESP_LOGD(TAG, "Temperature: %.1f °C", temperature);
  
  // Parse Humidity (bytes 9-10): Data[9]*256 + Data[10]
  // Unit is 0.1%RH
  uint16_t humidity_raw = (data[9] << 8) | data[10];
  float humidity = humidity_raw * 0.1f;
  if (this->humidity_sensor_ != nullptr) {
    this->humidity_sensor_->publish_state(humidity);
  }
  ESP_LOGD(TAG, "Humidity: %.1f %%", humidity);
  
  return true;
}

}  // namespace five_in_one_sensor
}  // namespace esphome
