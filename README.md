# ESPHome Sensor Components

This repository contains ESPHome external components for various environmental sensors.

## Available Components

### 1. 21VOC Sensor Component (`two_one_voc`)

ESPHome external component for a 21VOC environmental sensor module that measures:
- **VOC** (Volatile Organic Compounds) - Air quality in µg/m³
- **Formaldehyde** - Concentration in µg/m³
- **eCO2** - Equivalent CO2 in PPM
- **Temperature** - In °C with 0.1° precision
- **Humidity** - In %RH with 0.1% precision

**[Full documentation for 21VOC sensor](./ORIGINAL_21VOC_README.md)**

### 2. JX-CO2-102 Infrared CO2 Sensor Component (`jx_co2_102`)

ESPHome external component for the JX-CO2-102 series infrared CO2 sensor using NDIR technology that measures:
- **CO2** - Carbon dioxide concentration in PPM (0-5000/10000/30000/50000 ppm depending on model)

Features:
- High accuracy: ±50ppm or ±5% F.S at 25°C
- Fast response: <30s response time
- NDIR technology: Non-dispersive infrared detection
- Multiple range options: 5K/10K/30K/50K ppm models available

**[Full documentation for JX-CO2-102 sensor](./JX_CO2_102_README.md)**

### 3. PM2005 Laser Particle Sensor Component (`pm2005`)

ESPHome external component for the PM2005 laser particle sensor module that measures:
- **PM0.5** - Particle count for 0.5μm particles in PCS/L
- **PM2.5** - Particle count for 2.5μm particles in PCS/L
- **PM10** - Particle count for 10μm particles in PCS/L
- **PM2.5 Mass** - Mass concentration in μg/m³
- **PM10 Mass** - Mass concentration in μg/m³

Features:
- Smallest detectable particle: 0.3μm
- Accuracy: ±15% reading
- Fast response: 5 seconds
- Temperature range: 0-45°C (with full temperature correction)
- UART-TTL communication (9600 baud, 8N1)

**[Full documentation for PM2005 sensor](./PM2005_README.md)**

## Quick Start

### 21VOC Sensor

### Method 1: Using as External Component (Recommended)

Add this to your ESPHome YAML configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/component-esphome
      ref: main
    components: [ two_one_voc ]

uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

sensor:
  - platform: two_one_voc
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

### JX-CO2-102 Sensor

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/component-esphome
      ref: main
    components: [ jx_co2_102 ]

uart:
  rx_pin: GPIO16  # Connect to sensor TX pin
  baud_rate: 9600

sensor:
  - platform: jx_co2_102
    co2:
      name: "CO2 Concentration"
```

### PM2005 Sensor

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/component-esphome
      ref: main
    components: [ pm2005 ]

uart:
  tx_pin: GPIO17  # Connect to sensor RX pin
  rx_pin: GPIO16  # Connect to sensor TX pin
  baud_rate: 9600

sensor:
  - platform: pm2005
    pm_0_5:
      name: "PM0.5 Particles"
    pm_2_5:
      name: "PM2.5 Particles"
    pm_10_0:
      name: "PM10 Particles"
    pm_2_5_mass:
      name: "PM2.5 Mass"
    pm_10_0_mass:
      name: "PM10 Mass"
```

## Installation

### Method 1: Using as External Component (Recommended)

See Quick Start examples above for each sensor.

### Method 2: Local Installation

1. Clone this repository to your ESPHome configuration directory
2. Reference it as a local external component:

```yaml
external_components:
  - source:
      type: local
      path: path/to/component-esphome/components
    components: [ two_one_voc, jx_co2_102, pm2005 ]  # Choose components you need
```

## Detailed Documentation

For complete documentation, configuration options, troubleshooting, and technical details:

- **21VOC Sensor**: See [21VOC Documentation](./ORIGINAL_21VOC_README.md) or the sections below
- **JX-CO2-102 Sensor**: See [JX-CO2-102 Documentation](./JX_CO2_102_README.md)
- **PM2005 Sensor**: See [PM2005 Documentation](./PM2005_README.md)

---

# 21VOC Sensor - Detailed Information

## Hardware Specifications

- **Communication Protocol**: UART
- **Baud Rate**: 9600
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Data Format**: 12-byte packets sent actively by the module

## Configuration Example for 21VOC

```yaml
# UART configuration for the sensor
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

# 21VOC Sensor configuration
sensor:
  - platform: two_one_voc
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
  - platform: two_one_voc
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
