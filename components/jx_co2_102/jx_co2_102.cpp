#include "jx_co2_102.h"
#include "esphome/core/log.h"

namespace esphome {
namespace jx_co2_102 {

static const char *const TAG = "jx_co2_102";

void JXCO2102Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up JX-CO2-102 Sensor...");
}

void JXCO2102Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "JX-CO2-102 Infrared CO2 Sensor:");
  LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  this->check_uart_settings(9600);
}

void JXCO2102Sensor::loop() {
  // Read available data from UART
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // Add byte to buffer
    this->rx_buffer_.push_back(byte);
    
    // Check if we have a complete line (ending with \n)
    if (byte == '\n') {
      if (this->parse_ascii_data_()) {
        ESP_LOGV(TAG, "Successfully parsed CO2 data");
      } else {
        ESP_LOGW(TAG, "Invalid data packet received");
      }
      this->rx_buffer_.clear();
    }
  }
  
  // Clear buffer if it's too large (prevent memory overflow)
  if (this->rx_buffer_.size() > 20) {
    ESP_LOGW(TAG, "Buffer overflow, clearing");
    this->rx_buffer_.clear();
  }
}

bool JXCO2102Sensor::parse_ascii_data_() {
  // Expected format: "  xxxx ppm\r\n" or similar
  // Example: "  1235 ppm\r\n"
  // In hex: 20 20 31 32 33 35 20 70 70 6d 0D 0A
  
  if (this->rx_buffer_.empty()) {
    return false;
  }
  
  // Convert buffer to string
  std::string data_str(this->rx_buffer_.begin(), this->rx_buffer_.end());
  
  // Remove leading/trailing whitespace and control characters
  size_t start = data_str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return false;
  }
  
  size_t end = data_str.find_last_not_of(" \t\r\n");
  data_str = data_str.substr(start, end - start + 1);
  
  // Look for "ppm" in the string
  size_t ppm_pos = data_str.find("ppm");
  if (ppm_pos == std::string::npos) {
    ESP_LOGV(TAG, "No 'ppm' found in data");
    return false;
  }
  
  // Extract the numeric part before "ppm"
  std::string num_str = data_str.substr(0, ppm_pos);
  
  // Remove any remaining whitespace
  start = num_str.find_first_not_of(" \t");
  if (start == std::string::npos) {
    return false;
  }
  end = num_str.find_last_not_of(" \t");
  num_str = num_str.substr(start, end - start + 1);
  
  // Parse the number
  char *endptr;
  long parsed_value = strtol(num_str.c_str(), &endptr, 10);
  
  // Check if parsing was successful
  if (endptr == num_str.c_str() || *endptr != '\0') {
    ESP_LOGW(TAG, "Failed to parse CO2 value: '%s'", num_str.c_str());
    return false;
  }
  
  // Validate range (0-50000 ppm based on spec)
  // CO2 values cannot be negative, and max range is 50000 ppm
  if (parsed_value < 0 || parsed_value > 50000) {
    ESP_LOGW(TAG, "CO2 value out of range: %ld ppm", parsed_value);
    return false;
  }
  
  int co2_value = static_cast<int>(parsed_value);
  
  // Publish the value
  if (this->co2_sensor_ != nullptr) {
    this->co2_sensor_->publish_state(co2_value);
  }
  
  ESP_LOGD(TAG, "CO2: %d ppm", co2_value);
  
  return true;
}

}  // namespace jx_co2_102
}  // namespace esphome
