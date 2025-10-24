# PM2005 Laser Particle Sensor Component for ESPHome

ESPHome external component for the PM2005 laser particle sensor module.

## Features

- **Measurement Range**: PM0.3/PM2.5/PM10 (0.3μm to 10μm)
- **Accuracy**: ±15% reading
- **Response Time**: 5 seconds
- **Operating Temperature**: 0-45°C (full range temperature correction)
- **Storage Temperature**: -20 to +60°C
- **Operating Humidity**: 0-95% RH non-condensing
- **Power Supply**: 5.0 ±0.1 VDC; < 100mA
- **Communication**: UART-TTL (0-3.3V interface), also supports I2C and PWM

## Hardware Specifications

### General Parameters
- **Smallest Detectable Particle**: 0.3μm
- **Measurement Types**: Particle count (PCS/L) and mass concentration (μg/m³)
- **Communication Protocol**: UART-TTL (default), I2C, PWM (optional)
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Baud Rate**: 9600 bps

### Pin Definition (8-pin connector)
| Pin No. | Item | Description |
|---------|------|-------------|
| 1 | +3.3V | Power output (+3.3V/100mA) |
| 2 | 5V | Power input (5V) |
| 3 | SCL | I2C Clock |
| 4 | SDA | I2C Data |
| 5 | TEST | For testing |
| 6 | TX | UART-TX output (0-3.3V) |
| 7 | RX | UART-RX input (0-3.3V) |
| 8 | GND | Power input (ground terminal) |

## Installation

### Method 1: Using as External Component (Recommended)

Add this to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/component-esphome
      ref: main
    components: [ pm2005 ]
```

### Method 2: Local Installation

1. Clone this repository to your ESPHome configuration directory
2. Reference it as a local external component:

```yaml
external_components:
  - source:
      type: local
      path: path/to/component-esphome/components
    components: [ pm2005 ]
```

## Configuration Example

```yaml
# UART configuration for the sensor
uart:
  tx_pin: GPIO17  # Connect to sensor RX pin
  rx_pin: GPIO16  # Connect to sensor TX pin
  baud_rate: 9600

# PM2005 Sensor configuration
sensor:
  - platform: pm2005
    pm_0_5:
      name: "PM0.5 Particles"
      id: pm_0_5_sensor
    pm_2_5:
      name: "PM2.5 Particles"
      id: pm_2_5_sensor
    pm_10_0:
      name: "PM10 Particles"
      id: pm_10_0_sensor
    pm_2_5_mass:
      name: "PM2.5 Mass"
      id: pm_2_5_mass_sensor
    pm_10_0_mass:
      name: "PM10 Mass"
      id: pm_10_0_mass_sensor
```

## Configuration Options

All sensors are **optional**. You can configure only the sensors you need:

| Configuration | Type | Unit | Description |
|--------------|------|------|-------------|
| `pm_0_5` | Sensor | PCS/L | Particle count for 0.5μm particles |
| `pm_2_5` | Sensor | PCS/L | Particle count for 2.5μm particles |
| `pm_10_0` | Sensor | PCS/L | Particle count for 10μm particles |
| `pm_2_5_mass` | Sensor | μg/m³ | Mass concentration of PM2.5 |
| `pm_10_0_mass` | Sensor | μg/m³ | Mass concentration of PM10 |

Each sensor supports standard ESPHome sensor options like:
- `name` - Friendly name for the sensor
- `id` - Internal ID for the sensor
- `filters` - Apply filters like offset, calibrate_linear, etc.
- `on_value` - Trigger automations on value changes

## Hardware Connection

```
┌─────────────┐         ┌──────────────┐
│   PM2005    │         │     ESP32    │
│   Sensor    │         │              │
├─────────────┤         ├──────────────┤
│ Pin 2 (5V)  │────────▶│ 5V           │
│ Pin 8 (GND) │────────▶│ GND          │
│ Pin 6 (TX)  │────────▶│ GPIO16 (RX)  │
│ Pin 7 (RX)  │◀────────│ GPIO17 (TX)  │
└─────────────┘         └──────────────┘
```

**Important Notes:**
- Ensure power supply is 5.0V ±0.1V DC with at least 100mA capacity
- The sensor supports bidirectional UART communication
- Connect sensor TX to ESP RX and sensor RX to ESP TX
- The component automatically manages measurement cycles

## Communication Protocol

The PM2005 uses a custom binary protocol over UART:

### Command Format
| Byte | Description |
|------|-------------|
| 0 | Header (0x11 for commands, 0x16 for responses) |
| 1 | Length (number of data bytes + 1) |
| 2 | Command byte |
| 3-n | Data bytes |
| n+1 | Checksum (256 - sum of all previous bytes) |

### Key Commands
1. **Open/Close Measurement** (0x0C)
   - Opens particle measurement for 36 seconds
   - Component automatically handles this

2. **Read Particle Data** (0x0B)
   - Returns particle counts in PCS/L
   - Format: PM0.5, PM2.5, PM10
   - Also includes alarm and status information

3. **Read Mass Data** (0x0B with data=0x01)
   - Returns mass concentrations in μg/m³
   - Format: PM2.5 mass, PM10 mass

### Measurement Cycle
The component implements automatic measurement cycles:
1. Every 60 seconds, opens measurement
2. Waits 36 seconds for measurement to complete
3. Reads particle count data
4. Reads mass concentration data
5. Returns to idle state

## Data Interpretation

### Particle Count (PCS/L)
- **PM0.5**: Count of particles ≥0.5μm per liter of air
- **PM2.5**: Count of particles ≥2.5μm per liter of air
- **PM10**: Count of particles ≥10μm per liter of air

### Mass Concentration (μg/m³)
- **PM2.5 Mass**: Mass of particles ≤2.5μm per cubic meter
- **PM10 Mass**: Mass of particles ≤10μm per cubic meter
- Calculated using mathematical algorithms from particle counts

### Typical Values
- **Clean Air**: PM2.5 < 12 μg/m³, PM10 < 50 μg/m³
- **Moderate**: PM2.5 12-35 μg/m³, PM10 50-150 μg/m³
- **Unhealthy**: PM2.5 > 35 μg/m³, PM10 > 150 μg/m³

## Calibration

If you need to calibrate the readings, you can use ESPHome's filter feature:

```yaml
sensor:
  - platform: pm2005
    pm_2_5_mass:
      name: "PM2.5 Mass"
      filters:
        - calibrate_linear:
            - 0.0 -> 0.0
            - 100.0 -> 95.0  # Adjust based on reference sensor
    pm_10_0_mass:
      name: "PM10 Mass"
      filters:
        - offset: -5  # Apply offset if needed
```

## Troubleshooting

### No Data Received

1. Check UART connections (TX/RX pins must be crossed)
2. Verify baud rate is set to 9600
3. Ensure sensor has 5V power with sufficient current (≥100mA)
4. Check that power voltage is within 4.9V-5.1V range
5. Enable debug logging to see communication:
   ```yaml
   logger:
     level: DEBUG
   ```

### Invalid or Erratic Readings

- Ensure proper ventilation around air inlet and outlet
- Avoid placing sensor in areas with strong airflow
- Keep sensor away from heat sources
- Turn off the sensor inlet when not in use for extended periods
- Clean the sensor if operated in dusty environments

### Checksum Errors

- Check wiring for loose connections
- Verify power supply is stable
- Add a ground connection between ESP and sensor
- Check for electrical noise or interference

## Usage Precautions

1. **Application**: Designed for household electronics, not suitable for medical or mining equipment
2. **Materials**: Avoid static adsorption materials like metal plates near the sensor
3. **Environment**: Not suitable for very dusty environments
4. **Installation**: 
   - Ensure unobstructed air inlet and outlet
   - Avoid direct airflow to inlet/outlet
   - Install in correct orientation (see sensor documentation)
5. **Maintenance**: Turn off sampling inlet when not working for extended periods

## Technical Notes

1. The component automatically manages measurement cycles (60-second intervals)
2. Each measurement takes 36 seconds to complete
3. The component handles both particle count and mass data
4. Checksum validation ensures data integrity
5. The component is non-blocking and efficient
6. All sensor readings are optional - configure only what you need

## Applications

- Air purifiers
- Air quality monitoring instruments
- Fresh air systems
- Air conditioning systems
- Consumer electronics products
- Smart home air quality monitoring

## Advanced Features

The PM2005 protocol supports additional features not yet implemented:
- User correction coefficients
- Adjustable measurement time
- Timing measurement mode
- Dynamic measurement mode

If you need these features, please open an issue or submit a pull request.

## Known Limitations

- Current implementation uses UART only (I2C and PWM not supported)
- Manual calibration commands not implemented
- Fixed 60-second measurement interval (not configurable via ESPHome)
- Working condition alarms are read but not exposed as binary sensors

## License

This component is provided as-is for use with ESPHome.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Credits

Developed based on the PM2005 sensor module UART communication protocol documentation (V0.08).

## References

- ESPHome UART Component: https://esphome.io/components/uart.html
- ESPHome Sensor Component: https://esphome.io/components/sensor/
- ESPHome External Components: https://esphome.io/components/external_components.html
