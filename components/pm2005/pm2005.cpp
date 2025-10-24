#include "pm2005.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pm2005 {

static const char *const TAG = "pm2005";

void PM2005Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PM2005 Sensor...");
  this->state_ = PM2005_STATE_IDLE;
  this->measuring_ = false;
}

void PM2005Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PM2005 Laser Particle Sensor:");
  LOG_SENSOR("  ", "PM0.5", this->pm_0_5_sensor_);
  LOG_SENSOR("  ", "PM2.5", this->pm_2_5_sensor_);
  LOG_SENSOR("  ", "PM10", this->pm_10_0_sensor_);
  LOG_SENSOR("  ", "PM2.5 Mass", this->pm_2_5_mass_sensor_);
  LOG_SENSOR("  ", "PM10 Mass", this->pm_10_0_mass_sensor_);
  this->check_uart_settings(9600);
}

void PM2005Sensor::loop() {
  // Read available data from UART
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // Add byte to buffer
    this->rx_buffer_.push_back(byte);
    
    // Check if we have enough data for a response
    if (this->rx_buffer_.size() >= PM2005_RESP_HEADER_LEN) {
      // Check for valid header
      if (this->rx_buffer_[0] == PM2005_RESP_HEADER) {
        uint8_t expected_len = this->rx_buffer_[1] + 3;  // LEN + HEADER + LEN + CS
        
        // Wait for complete packet
        if (this->rx_buffer_.size() >= expected_len) {
          if (this->parse_response_()) {
            ESP_LOGV(TAG, "Successfully parsed response");
          } else {
            ESP_LOGW(TAG, "Invalid response packet received");
          }
          this->rx_buffer_.clear();
        }
      } else {
        // Invalid header, shift buffer
        this->rx_buffer_.erase(this->rx_buffer_.begin());
      }
    }
  }
  
  // Clear buffer if it's too large (prevent memory overflow)
  if (this->rx_buffer_.size() > 50) {
    ESP_LOGW(TAG, "Buffer overflow, clearing");
    this->rx_buffer_.clear();
  }

  // State machine for periodic measurements
  uint32_t now = millis();
  
  switch (this->state_) {
    case PM2005_STATE_IDLE:
      // Start measurement every 60 seconds
      if (now - this->last_measurement_time_ > 60000) {
        this->open_measurement_();
        this->state_ = PM2005_STATE_WAIT_RESPONSE;
        this->last_command_time_ = now;
        this->last_measurement_time_ = now;
      }
      break;
      
    case PM2005_STATE_WAIT_RESPONSE:
      // Wait for response (timeout after 1 second)
      if (now - this->last_command_time_ > 1000) {
        ESP_LOGW(TAG, "Command timeout, returning to idle");
        this->state_ = PM2005_STATE_IDLE;
      }
      break;
      
    case PM2005_STATE_MEASURING:
      // Read particle data after 36 seconds of measurement
      if (now - this->last_command_time_ > 36000) {
        this->read_particle_data_();
        this->last_command_time_ = now;
        this->read_mass_next_ = false;
      }
      // After reading particle data, wait a bit then read mass data
      else if (!this->read_mass_next_ && now - this->last_command_time_ > 500) {
        this->read_mass_data_();
        this->read_mass_next_ = true;
        this->last_command_time_ = now;
      }
      // After reading mass data, wait a bit then return to idle
      else if (this->read_mass_next_ && now - this->last_command_time_ > 500) {
        this->state_ = PM2005_STATE_IDLE;
      }
      break;
  }
}

void PM2005Sensor::send_command_(uint8_t cmd, const uint8_t *data, uint8_t data_len) {
  uint8_t buffer[32];
  uint8_t idx = 0;
  
  buffer[idx++] = PM2005_CMD_HEADER;
  buffer[idx++] = data_len + 1;  // Length includes CMD
  buffer[idx++] = cmd;
  
  for (uint8_t i = 0; i < data_len; i++) {
    buffer[idx++] = data[i];
  }
  
  // Calculate checksum: 256 - (sum of all bytes except checksum)
  uint8_t sum = 0;
  for (uint8_t i = 0; i < idx; i++) {
    sum += buffer[i];
  }
  buffer[idx++] = (256 - sum) & 0xFF;
  
  // Send command
  this->write_array(buffer, idx);
  this->flush();
  
  ESP_LOGV(TAG, "Sent command 0x%02X with %d data bytes", cmd, data_len);
}

void PM2005Sensor::open_measurement_() {
  ESP_LOGD(TAG, "Opening measurement");
  uint8_t data[] = {0x02, 0x1E};  // Open measurement
  this->send_command_(PM2005_CMD_OPEN_CLOSE, data, 2);
}

void PM2005Sensor::read_particle_data_() {
  ESP_LOGD(TAG, "Reading particle data");
  this->send_command_(PM2005_CMD_READ_PARTICLE, nullptr, 0);
}

void PM2005Sensor::read_mass_data_() {
  ESP_LOGD(TAG, "Reading mass data");
  uint8_t data[] = {0x01};  // Read mass data
  this->send_command_(PM2005_CMD_READ_MASS, data, 1);
}

bool PM2005Sensor::parse_response_() {
  if (this->rx_buffer_.size() < PM2005_RESP_HEADER_LEN) {
    return false;
  }
  
  // Check header
  if (this->rx_buffer_[0] != PM2005_RESP_HEADER) {
    ESP_LOGW(TAG, "Invalid response header: 0x%02X", this->rx_buffer_[0]);
    return false;
  }
  
  uint8_t len = this->rx_buffer_[1];
  uint8_t cmd = this->rx_buffer_[2];
  uint8_t expected_total_len = len + 3;  // LEN + HEADER + LEN + CS
  
  if (this->rx_buffer_.size() < expected_total_len) {
    return false;  // Not enough data yet
  }
  
  // Verify checksum
  uint8_t sum = 0;
  for (uint8_t i = 0; i < expected_total_len - 1; i++) {
    sum += this->rx_buffer_[i];
  }
  uint8_t expected_cs = (256 - sum) & 0xFF;
  uint8_t received_cs = this->rx_buffer_[expected_total_len - 1];
  
  if (expected_cs != received_cs) {
    ESP_LOGW(TAG, "Checksum mismatch: expected 0x%02X, got 0x%02X", expected_cs, received_cs);
    return false;
  }
  
  // Process response based on command
  if (cmd == PM2005_CMD_OPEN_CLOSE) {
    // Response to open/close command
    if (len >= 2 && this->rx_buffer_[3] == 0x02) {
      ESP_LOGD(TAG, "Measurement opened successfully");
      this->state_ = PM2005_STATE_MEASURING;
      this->last_command_time_ = millis();
      this->measuring_ = true;
    }
    return true;
  } else if (cmd == PM2005_CMD_READ_PARTICLE && len >= 17) {
    // Parse particle data (PCS/L)
    // Response format: 16 11 0B DF1 DF2 DF3 DF4 DF5 DF6 DF7 DF8 DF9 DF10 DF11 DF12 DF13 DF14 DF15 DF16 [CS]
    // 0.5um: DF1-DF4, 2.5um: DF5-DF8, 10um: DF9-DF12
    
    uint32_t pm_0_5 = (uint32_t)this->rx_buffer_[3] << 24 | 
                      (uint32_t)this->rx_buffer_[4] << 16 | 
                      (uint32_t)this->rx_buffer_[5] << 8 | 
                      (uint32_t)this->rx_buffer_[6];
    
    uint32_t pm_2_5 = (uint32_t)this->rx_buffer_[7] << 24 | 
                      (uint32_t)this->rx_buffer_[8] << 16 | 
                      (uint32_t)this->rx_buffer_[9] << 8 | 
                      (uint32_t)this->rx_buffer_[10];
    
    uint32_t pm_10_0 = (uint32_t)this->rx_buffer_[11] << 24 | 
                       (uint32_t)this->rx_buffer_[12] << 16 | 
                       (uint32_t)this->rx_buffer_[13] << 8 | 
                       (uint32_t)this->rx_buffer_[14];
    
    ESP_LOGD(TAG, "PM0.5: %u PCS/L, PM2.5: %u PCS/L, PM10: %u PCS/L", pm_0_5, pm_2_5, pm_10_0);
    
    // Publish values
    if (this->pm_0_5_sensor_ != nullptr) {
      this->pm_0_5_sensor_->publish_state(pm_0_5);
    }
    if (this->pm_2_5_sensor_ != nullptr) {
      this->pm_2_5_sensor_->publish_state(pm_2_5);
    }
    if (this->pm_10_0_sensor_ != nullptr) {
      this->pm_10_0_sensor_->publish_state(pm_10_0);
    }
    
    return true;
  } else if (cmd == PM2005_CMD_READ_MASS && len >= 17) {
    // Parse mass data (μg/m³)
    // PM2.5: DF1-DF4, PM10: DF5-DF8
    
    uint32_t pm_2_5_mass = (uint32_t)this->rx_buffer_[3] << 24 | 
                           (uint32_t)this->rx_buffer_[4] << 16 | 
                           (uint32_t)this->rx_buffer_[5] << 8 | 
                           (uint32_t)this->rx_buffer_[6];
    
    uint32_t pm_10_0_mass = (uint32_t)this->rx_buffer_[7] << 24 | 
                            (uint32_t)this->rx_buffer_[8] << 16 | 
                            (uint32_t)this->rx_buffer_[9] << 8 | 
                            (uint32_t)this->rx_buffer_[10];
    
    ESP_LOGD(TAG, "PM2.5 Mass: %u μg/m³, PM10 Mass: %u μg/m³", pm_2_5_mass, pm_10_0_mass);
    
    // Publish values
    if (this->pm_2_5_mass_sensor_ != nullptr) {
      this->pm_2_5_mass_sensor_->publish_state(pm_2_5_mass);
    }
    if (this->pm_10_0_mass_sensor_ != nullptr) {
      this->pm_10_0_mass_sensor_->publish_state(pm_10_0_mass);
    }
    
    return true;
  }
  
  return false;
}

uint8_t PM2005Sensor::calculate_checksum_(const uint8_t *data, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return (256 - sum) & 0xFF;
}

}  // namespace pm2005
}  // namespace esphome
