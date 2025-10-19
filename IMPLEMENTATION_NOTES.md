# Implementation Notes

## Overview

This repository contains an ESPHome external component for a 5-in-1 environmental sensor module. The component implements the serial protocol specified in the Chinese documentation.

## Protocol Details

### Communication Parameters
- **Baud Rate**: 9600
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None (8N1)

### Data Packet Structure

The sensor sends 12-byte packets automatically:

```
Byte  0: Header (0x2C)
Byte  1: VOC High Byte
Byte  2: VOC Low Byte
Byte  3: Formaldehyde High Byte
Byte  4: Formaldehyde Low Byte
Byte  5: eCO2 High Byte
Byte  6: eCO2 Low Byte
Byte  7: Temperature High Byte
Byte  8: Temperature Low Byte
Byte  9: Humidity High Byte
Byte 10: Humidity Low Byte
Byte 11: Checksum
```

### Data Parsing

#### 16-bit Values
All sensor values (except checksum) are transmitted as 16-bit big-endian values:
```
Value = (HighByte × 256) + LowByte
```

**Note**: The original specification mentioned "Data[X]*28+Data[Y]" which appears to be a transcription error. The standard and correct formula for combining two bytes is "Data[X]*256+Data[Y]".

#### Temperature Encoding

Temperature uses a special encoding for negative values:

1. **Positive temperatures**: Direct 16-bit unsigned value
   - Example: 31.7°C = 317 (0x013D)
   
2. **Negative temperatures**: MSB (bit 15) is 1
   - Formula: `actual_value = -(0xFFFF - raw_value)`
   - Example: -10°C
     - Raw value: 0xFFF5
     - Calculation: 0xFFFF - 0xFFF5 = 0x000A = 10
     - Result: -10°C

3. **Final conversion**: Multiply by 0.1 to get actual temperature
   - Example: 317 × 0.1 = 31.7°C

#### Humidity Encoding

Humidity is transmitted as a 16-bit unsigned value in 0.1%RH units:
```
Humidity (%) = raw_value × 0.1
```

#### Checksum Validation

The checksum is a two's complement checksum:
```
Checksum = (~sum_of_bytes_0_to_10) + 1
```

This ensures data integrity during transmission.

## Implementation Details

### C++ Component (`five_in_one_sensor.cpp` / `.h`)

The component inherits from:
- `uart::UARTDevice` - For UART communication
- `Component` - For ESPHome lifecycle management

Key methods:
- `loop()` - Reads UART data, synchronizes on header byte, collects 12-byte packets
- `parse_data_()` - Validates and parses complete packets
- `validate_checksum_()` - Verifies packet integrity
- `parse_temperature_()` - Handles negative temperature encoding

### Python Configuration (`sensor.py`)

The Python configuration file:
- Defines the sensor schema with all optional sensors
- Sets appropriate units and device classes
- Configures icons for each sensor type
- Registers the component with ESPHome

### Design Decisions

1. **Non-blocking operation**: The `loop()` method processes available data without blocking
2. **Automatic synchronization**: Uses header byte (0x2C) to synchronize packet boundaries
3. **Optional sensors**: All sensors are optional - users configure only what they need
4. **Robust error handling**: Invalid checksums and malformed packets are logged and discarded
5. **Buffer management**: Automatically clears buffer on overflow to prevent memory issues

## Testing Recommendations

When testing this component:

1. **Verify UART connections**: Ensure RX/TX pins are correctly connected
2. **Check data flow**: Enable DEBUG logging to see raw packet data
3. **Validate checksums**: Monitor for checksum errors indicating noise/interference
4. **Test negative temperatures**: If possible, test with cold temperatures to verify negative value handling
5. **Calibration**: Use ESPHome filters to apply offset corrections if needed due to PCB heating

## Future Enhancements

Potential improvements for future versions:

1. **Configurable update interval**: Currently processes every packet; could add throttling
2. **Data smoothing**: Add moving average filters in the component
3. **Sensor health monitoring**: Track consecutive checksum failures
4. **Power management**: Add sleep/wake commands if supported by hardware

## References

- ESPHome UART Component: https://esphome.io/components/uart.html
- ESPHome Sensor Component: https://esphome.io/components/sensor/
- ESPHome External Components: https://esphome.io/components/external_components.html
