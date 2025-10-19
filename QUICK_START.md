# Quick Start Guide

## Installation (2 minutes)

### Step 1: Add to your ESPHome YAML

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lyj0309/21voc-esphome
      ref: main
    components: [ five_in_one_sensor ]

uart:
  tx_pin: GPIO17  # Adjust to your setup
  rx_pin: GPIO16  # Adjust to your setup
  baud_rate: 9600

sensor:
  - platform: five_in_one_sensor
    voc:
      name: "Air Quality VOC"
    formaldehyde:
      name: "Formaldehyde"
    eco2:
      name: "CO2"
    temperature:
      name: "Temperature"
    humidity:
      name: "Humidity"
```

### Step 2: Upload to your ESP device

```bash
esphome run your-config.yaml
```

### Step 3: Check logs

```bash
esphome logs your-config.yaml
```

You should see readings like:
```
[D][five_in_one_sensor:xxx]: VOC: 123 µg/m³
[D][five_in_one_sensor:xxx]: Formaldehyde: 45 µg/m³
[D][five_in_one_sensor:xxx]: eCO2: 678 PPM
[D][five_in_one_sensor:xxx]: Temperature: 23.5 °C
[D][five_in_one_sensor:xxx]: Humidity: 55.2 %
```

## Hardware Connection

```
┌─────────────┐         ┌──────────────┐
│   Sensor    │         │     ESP32    │
│   Module    │         │              │
├─────────────┤         ├──────────────┤
│ VCC (5V)    │────────▶│ 5V           │
│ GND         │────────▶│ GND          │
│ TX          │────────▶│ GPIO16 (RX)  │
│ RX          │◀────────│ GPIO17 (TX)  │ (not needed for read-only)
└─────────────┘         └──────────────┘
```

**Note:** The sensor actively sends data, so you only need to connect the sensor's TX to ESP's RX.

## Common Issues

### No data appearing?
1. Check wiring (especially TX→RX connection)
2. Verify baud rate is 9600
3. Enable debug logging: `logger: level: DEBUG`

### Checksum errors?
- Check ground connection
- Add a 10kΩ pull-up resistor on RX line
- Check for electrical noise/interference

### Need calibration?
Use ESPHome filters:
```yaml
sensor:
  - platform: five_in_one_sensor
    temperature:
      name: "Temperature"
      filters:
        - offset: -2.0  # Adjust as needed
```

## Optional Sensors

You don't need to configure all sensors. Only add what you need:

```yaml
sensor:
  - platform: five_in_one_sensor
    temperature:
      name: "Temperature"
    humidity:
      name: "Humidity"
    # VOC, formaldehyde, and eCO2 not configured
```

## More Information

- **Full documentation**: See [README.md](README.md)
- **Technical details**: See [IMPLEMENTATION_NOTES.md](IMPLEMENTATION_NOTES.md)
- **Packet format**: See [PACKET_FORMAT.md](PACKET_FORMAT.md)
- **Complete example**: See [example.yaml](example.yaml)

## Support

If you encounter issues:
1. Enable debug logging
2. Check the documentation
3. Open an issue on GitHub with your logs and configuration
