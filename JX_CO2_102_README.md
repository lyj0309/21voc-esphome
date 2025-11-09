# JX-CO2-102 Infrared CO2 Sensor Component for ESPHome

ESPHome external component for the JX-CO2-102 series infrared CO2 sensor module using NDIR (Non-Dispersive Infrared) technology.

## Features

- **High Accuracy**: ±50ppm or ±5% F.S at 25°C
- **Wide Range**: Multiple models supporting 0-5000ppm up to 0-50000ppm
- **Fast Response**: <30s response time at 25°C
- **NDIR Technology**: Non-dispersive infrared detection
- **Temperature Compensation**: Built-in temperature compensation (0-50°C)
- **Multiple Interfaces**: UART, PWM, and analog voltage output

## Hardware Specifications

### General Parameters
- **Measurement Range**: 0-5000ppm (default, other ranges available)
- **Resolution**: 1ppm
- **Accuracy**: ±50ppm or ±5% F.S at 25°C
- **Response Time**: <30s at 25°C
- **Warm-up Time**: <1min at 25°C (full accuracy in <5min)
- **Operating Voltage**: 5V DC
- **Current Consumption**: <60mA (average)
- **Operating Humidity**: 0-95%RH (non-condensing)
- **Operating Temperature**: -10~60°C (default compensation: 0-50°C)
- **Storage Temperature**: -20~70°C

### Available Models
| Model | Measurement Range |
|-------|------------------|
| JX-CO2-102-5K | 0-5000ppm |
| JX-CO2-102-1W | 0-10000ppm |
| JX-CO2-102-3W | 0-30000ppm |
| JX-CO2-102-5W | 0-50000ppm |

### Pin Definition
| Pin | Name | Description |
|-----|------|-------------|
| 1 | PWM | PWM output (CO2 concentration) |
| 2 | TX | TTL level signal, 3.3V |
| 3 | RX | TTL level signal, 3.3V |
| 4 | 5V | Power supply 5V |
| 5 | GND | Ground |
| 6 | DA | Analog voltage output |
| 7 | HD | Reserved |

**Note**: The sensor outputs full scale reading for first 90 seconds after power-on, then returns to normal operation.

## Communication Protocols

The JX-CO2-102 supports three communication modes:

### 1. Active ASCII Reporting Mode (Default - Implemented)
The sensor automatically sends CO2 concentration every 1 second in ASCII format:
- **Format**: `"  xxxx ppm\r\n"`
- **Example**: `"  1235 ppm\r\n"` (spaces + value + space + "ppm" + CR + LF)
- **Baud Rate**: 9600
- **This is the mode currently implemented in this component**

### 2. Passive Query Mode (Custom Protocol)
Query the sensor on demand using custom commands.

### 3. MODBUS-RTU Mode
Standard MODBUS-RTU protocol for querying CO2 values.

## Installation

### Method 1: Using as External Component (Recommended)

Add this to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/component-esphome
      ref: main
    components: [ jx_co2_102 ]
```

### Method 2: Local Installation

1. Clone this repository to your ESPHome configuration directory
2. Reference it as a local external component:

```yaml
external_components:
  - source:
      type: local
      path: path/to/component-esphome/components
    components: [ jx_co2_102 ]
```

## Configuration Example

```yaml
# UART configuration for the sensor
uart:
  tx_pin: GPIO17  # Optional - sensor only transmits
  rx_pin: GPIO16  # Connect to sensor's TX pin
  baud_rate: 9600

# JX-CO2-102 Sensor configuration
sensor:
  - platform: jx_co2_102
    co2:
      name: "CO2 Concentration"
      id: co2_sensor
```

## Configuration Options

| Configuration | Type | Description |
|--------------|------|-------------|
| `co2` | Sensor | CO2 concentration sensor in ppm |

The CO2 sensor supports standard ESPHome sensor options like:
- `name` - Friendly name for the sensor
- `id` - Internal ID for the sensor
- `filters` - Apply filters like offset, calibrate_linear, etc.
- `on_value` - Trigger automations on value changes

## Hardware Connection

```
┌─────────────┐         ┌──────────────┐
│  JX-CO2-102 │         │     ESP32    │
│   Sensor    │         │              │
├─────────────┤         ├──────────────┤
│ Pin 4 (5V)  │────────▶│ 5V           │
│ Pin 5 (GND) │────────▶│ GND          │
│ Pin 2 (TX)  │────────▶│ GPIO16 (RX)  │
└─────────────┘         └──────────────┘
```

**Important Notes:**
- The sensor transmits data actively, so you only need to connect sensor TX to ESP RX
- Ensure power supply is between 4.5V-5.5V DC with at least 150mA capacity
- Allow 1 minute warm-up time and 5 minutes for full accuracy

## Data Format

In Active ASCII Reporting Mode (default):
- The sensor sends data every 1 second
- Format: ASCII string `"  xxxx ppm\r\n"` where xxxx is the CO2 value
- Example hex data for 1235 ppm: `20 20 31 32 33 35 20 70 70 6d 0D 0A`
  - `0x20` = space character
  - `0x31 0x32 0x33 0x35` = "1235" in ASCII
  - `0x70 0x70 0x6d` = "ppm" in ASCII
  - `0x0D 0x0A` = CR LF (line ending)

## Calibration

### Important Calibration Information

The sensor supports two calibration modes for zeroing at 400ppm (outdoor ambient CO2 level):

#### 1. Manual Quick Calibration (Now Supported!)
This component now supports manual calibration directly from ESPHome!

**Prerequisites:**
- Place sensor outdoors or in well-ventilated area (ambient CO2 ≈ 400ppm)
- Allow 10 minutes for warm-up and gas diffusion
- Ensure sensor has been running continuously for at least 30 minutes

**How to Use:**

Add a button to your ESPHome configuration:

```yaml
button:
  - platform: template
    name: "Calibrate CO2 Sensor"
    on_press:
      - jx_co2_102.calibrate_zero:
          id: co2_sensor_id
```

Or trigger it via an automation:

```yaml
sensor:
  - platform: jx_co2_102
    id: co2_sensor_id
    co2:
      name: "CO2"

button:
  - platform: template
    name: "Calibrate CO2 to 400ppm"
    on_press:
      - jx_co2_102.calibrate_zero: co2_sensor_id
```

**Technical Details:**
- Command sent: `FF 01 05 07 00 00 00 00 F4`
- Success response: `FF 01 03 07 01 00 00 00 F5`
- Calibrates sensor to 400ppm reference point
- Suitable for: Office and home environments
- **Not suitable for**: Greenhouses, farms, cold storage, etc.

#### 2. 24-Hour Automatic Calibration
- Automatically calibrates once every 24 hours
- Disabled by default
- Best for long-term outdoor installations
- Must be enabled via UART command (requires additional implementation)

**Recommendation**: Manually calibrate the sensor every 6 months for best accuracy using the calibrate_zero action.

## PWM Output (Alternative Interface)

If using PWM instead of UART:
- **Period**: 1004ms ±5%
- **Formula**: Positive pulse width = (PPM concentration / 5) + 2ms
- **Example for 0-5000ppm range**:
  - 0 ppm: 2ms pulse width
  - 2500 ppm: 502ms pulse width
  - 5000 ppm: 1002ms pulse width

## Analog Voltage Output (Alternative Interface)

The DA pin provides analog voltage output (customizable range):
- **Voltage Range**: 0-3.0V
- **Example mappings**:
  - 0.4-2V corresponds to 0-2000ppm
  - 0-3.0V corresponds to 0-5000ppm
- Contact manufacturer for custom range configuration

## Troubleshooting

### No Data Received

1. Check UART connections (sensor TX to ESP RX)
2. Verify baud rate is set to 9600
3. Ensure sensor has 5V power with sufficient current (≥150mA)
4. Wait for 90-second startup period
5. Enable debug logging to see raw data:
   ```yaml
   logger:
     level: DEBUG
   ```

### Invalid or Erratic Readings

- Allow 5 minutes for sensor to reach full accuracy after power-on
- Check that power supply is stable (4.5V-5.5V DC)
- Ensure good ventilation around sensor diffusion windows
- Avoid placing sensor near heat sources or in direct sunlight
- Clean sensor if operated in dusty environment

### Calibration Issues

- Use ESPHome filters for software calibration:
  ```yaml
  sensor:
    - platform: jx_co2_102
      co2:
        name: "CO2"
        filters:
          - offset: -50  # Adjust as needed
          - calibrate_linear:
              - 400 -> 400   # Outdoor ambient level
              - 1000 -> 1000
  ```

## Usage Precautions

1. **Avoid Pressure**: Do not apply pressure to the aluminum alloy tube cavity during soldering, installation, or use
2. **Ventilation**: Ensure both diffusion windows are in well-ventilated positions
3. **Heat**: Keep away from heat sources and avoid direct sunlight/thermal radiation
4. **Calibration**: Calibrate every 6 months (recommended)
5. **Dust**: Avoid long-term use in high-dust environments
6. **Power**: Maintain 4.5V-5.5V DC with ≥150mA current; outside this range may cause malfunction

## Technical Notes

1. The component automatically parses ASCII format data from the sensor
2. Data validation ensures readings are within valid range (0-50000 ppm)
3. The component is non-blocking and efficient
4. Buffer overflow protection prevents memory issues

## Applications

- HVAC fresh air control systems
- Indoor air quality monitoring
- Agriculture and livestock production monitoring
- Smart buildings and ventilation systems
- Wall-mounted controllers
- Robots and automotive applications

## Advanced Features (Not Yet Implemented)

The following features require additional protocol implementation:

- Switching between communication modes (ASCII/Query/MODBUS)
- Sending calibration commands
- Changing sensor address
- Changing baud rate
- Reading via MODBUS-RTU

If you need these features, please open an issue or submit a pull request.

## License

This component is provided as-is for use with ESPHome.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Credits

Developed based on the JX-CO2-102 sensor module documentation (Ver1.0).

## References

- ESPHome UART Component: https://esphome.io/components/uart.html
- ESPHome Sensor Component: https://esphome.io/components/sensor/
- ESPHome External Components: https://esphome.io/components/external_components.html
