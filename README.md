# 21VOC Sensor Component for ESPHome

ESPHome external component for a 21VOC environmental sensor module that measures:
- **VOC** (Volatile Organic Compounds) - Air quality in µg/m³
- **Formaldehyde** - Concentration in µg/m³
- **eCO2** - Equivalent CO2 in PPM
- **Temperature** - In °C with 0.1° precision
- **Humidity** - In %RH with 0.1% precision

## Hardware Specifications

- **Communication Protocol**: UART
- **Baud Rate**: 9600
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Data Format**: 12-byte packets sent actively by the module

## Installation

### Method 1: Using as External Component (Recommended)

Add this to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/21voc-esphome
      ref: main
    components: [ five_in_one_sensor ]
```

### Method 2: Local Installation

1. Clone this repository to your ESPHome configuration directory
2. Reference it as a local external component:

```yaml
external_components:
  - source:
      type: local
      path: path/to/21voc-esphome/components
    components: [ five_in_one_sensor ]
```

## Configuration Example

```yaml
# UART configuration for the sensor
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

# 21VOC Sensor configuration
sensor:
  - platform: five_in_one_sensor
    voc:
      name: "VOC Air Quality"
    formaldehyde:
      name: "Formaldehyde"
    eco2:
      name: "eCO2"
    temperature:
      name: "Temperature"
    humidity:
      name: "Humidity"
```

## Configuration Options

All sensors are **optional**. You can configure only the sensors you need:

| Configuration | Type | Description |
|--------------|------|-------------|
| `voc` | Sensor | VOC air quality sensor in µg/m³ |
| `formaldehyde` | Sensor | Formaldehyde concentration in µg/m³ |
| `eco2` | Sensor | Equivalent CO2 in PPM |
| `temperature` | Sensor | Temperature in °C (supports negative values) |
| `humidity` | Sensor | Relative humidity in % |

Each sensor supports standard ESPHome sensor options like:
- `name` - Friendly name for the sensor
- `id` - Internal ID for the sensor
- `filters` - Apply filters like offset, calibrate_linear, etc.
- `on_value` - Trigger automations on value changes

## Data Packet Format

The module sends 12-byte data packets with the following structure:

| Byte | Description | Formula |
|------|-------------|---------|
| 0 | Header | Always 0x2C |
| 1-2 | VOC (µg/m³) | Data[1] × 256 + Data[2] |
| 3-4 | Formaldehyde (µg/m³) | Data[3] × 256 + Data[4] |
| 5-6 | eCO2 (PPM) | Data[5] × 256 + Data[6] |
| 7-8 | Temperature (0.1°) | Data[7] × 256 + Data[8] |
| 9-10 | Humidity (0.1%RH) | Data[9] × 256 + Data[10] |
| 11 | Checksum | (~sum(0-10)) + 1 |

### Temperature Handling

The temperature value uses special encoding for negative values:
- If the MSB (bit 15) is 1, the temperature is negative
- Negative values are calculated as: `magnitude = 0xFFFF - raw_value`, then apply negative sign
- The final temperature is multiplied by 0.1 to get the actual value in °C

**Example:**
- Raw value: 0xFFF5
- Calculation: 0xFFFF - 0xFFF5 = 0x000A = 10 units
- With negative sign: -10 units
- Actual temperature: -10 × 0.1 = -1.0°C

**For -10.0°C:**
- Need: -100 units (since -100 × 0.1 = -10.0°C)
- Encoded as: 0xFFFF - 100 = 0xFF9B

### Calibration

If you need to calibrate the temperature or humidity readings due to PCB heating or environmental factors, you can use ESPHome's filter feature:

```yaml
sensor:
  - platform: five_in_one_sensor
    temperature:
      name: "Temperature"
      filters:
        - offset: -2.0  # Subtract 2°C if the sensor reads high
    humidity:
      name: "Humidity"
      filters:
        - calibrate_linear:
            - 0.0 -> 0.0
            - 100.0 -> 100.0
```

## Troubleshooting

### No Data Received

1. Check UART connections (TX/RX pins)
2. Verify baud rate is set to 9600
3. Enable debug logging to see raw data:
   ```yaml
   logger:
     level: DEBUG
   ```

### Invalid Checksum Errors

- This usually indicates electrical noise or connection issues
- Check wiring and add a ground connection
- Try adding a pull-up resistor on the RX line

### Unexpected Values

- Use the `filters` option to apply offset or calibration
- Check if temperature compensation is needed due to PCB heating

## Technical Notes

1. The component automatically handles packet synchronization using the header byte (0x2C)
2. Checksum validation ensures data integrity
3. The component is designed to be non-blocking and efficient
4. All sensor readings are optional - configure only what you need

## License

This component is provided as-is for use with ESPHome.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Credits

Developed based on the 21VOC sensor module serial protocol documentation.
