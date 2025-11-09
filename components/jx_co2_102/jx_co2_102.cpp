#include "jx_co2_102.h"
#include "esphome/core/log.h"

namespace esphome {
namespace jx_co2_102 {

static const char *const TAG = "jx_co2_102";

// Command to start manual quick calibration (calibrate to 400ppm)
// Format: FF 01 05 07 00 00 00 00 F4
static const uint8_t JX_CO2_CMD_CALIBRATE[9] = {0xFF, 0x01, 0x05, 0x07, 0x00, 0x00, 0x00, 0x00, 0xF4};

// Expected response for successful calibration
// Format: FF 01 03 07 01 00 00 00 F5
static const uint8_t JX_CO2_CALIBRATE_RESPONSE[9] = {0xFF, 0x01, 0x03, 0x07, 0x01, 0x00, 0x00, 0x00, 0xF5};

void JXCO2102Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up JX-CO2-102 Sensor...");
}

void JXCO2102Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "JX-CO2-102 Infrared CO2 Sensor:");
  LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  this->check_uart_settings(9600);
}

void JXCO2102Sensor::update() {
  // This is called periodically by PollingComponent
  // For this sensor, we don't need to do anything here as data comes in automatically
  // The loop() method handles incoming data
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

uint8_t JXCO2102Sensor::jx_co2_checksum_(const uint8_t *data, uint8_t len) {
  // Calculate checksum for JX-CO2-102 command packets
  // The checksum is the last byte and should make the sum of all bytes equal to 0x00
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len - 1; i++) {
    sum += data[i];
  }
  return (0x100 - sum) & 0xFF;
}

bool JXCO2102Sensor::jx_co2_write_command_(const uint8_t *command, uint8_t command_len, uint8_t *response,
                                            uint8_t response_len) {
  // Empty RX Buffer first
  while (this->available())
    this->read();

  // Write command to UART
  this->write_array(command, command_len);
  this->flush();

  if (response == nullptr)
    return true;

  // Wait for response with timeout
  uint32_t start_time = millis();
  uint8_t pos = 0;

  while (pos < response_len) {
    if (millis() - start_time > 1000) {  // 1 second timeout
      ESP_LOGW(TAG, "Timeout waiting for response");
      return false;
    }

    if (this->available()) {
      this->read_byte(&response[pos]);
      pos++;
    } else {
      delay(10);
    }
  }

  return true;
}

void JXCO2102Sensor::calibrate_zero() {
  ESP_LOGI(TAG, "Starting manual calibration to 400ppm...");
  ESP_LOGI(TAG, "Please ensure sensor has been running for 10+ minutes in outdoor/well-ventilated area");

  uint8_t response[9] = {0};

  if (!this->jx_co2_write_command_(JX_CO2_CMD_CALIBRATE, sizeof(JX_CO2_CMD_CALIBRATE), response,
                                     sizeof(response))) {
    ESP_LOGW(TAG, "Failed to send calibration command!");
    this->status_set_warning();
    return;
  }

  // Check if correct response received
  bool success = true;
  for (uint8_t i = 0; i < sizeof(response); i++) {
    if (response[i] != JX_CO2_CALIBRATE_RESPONSE[i]) {
      success = false;
      break;
    }
  }

  if (!success) {
    ESP_LOGW(TAG, "Got wrong response from JX-CO2-102. Expected: FF 01 03 07 01 00 00 00 F5");
    ESP_LOGW(TAG, "Got: %02X %02X %02X %02X %02X %02X %02X %02X %02X", response[0], response[1], response[2],
             response[3], response[4], response[5], response[6], response[7], response[8]);
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  ESP_LOGI(TAG, "JX-CO2-102 calibration successful! Sensor calibrated to 400ppm");
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
