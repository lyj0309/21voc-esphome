# Data Packet Format Reference

## Packet Structure

```
┌─────────┬─────────────┬───────────────────────────────────────────────┐
│  Byte   │    Hex      │                 Description                   │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    0    │    0x2C     │  Header (always 0x2C for sync)                │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    1    │    High     │  VOC Air Quality High Byte                    │
│    2    │    Low      │  VOC Air Quality Low Byte → µg/m³             │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    3    │    High     │  Formaldehyde High Byte                       │
│    4    │    Low      │  Formaldehyde Low Byte → µg/m³                │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    5    │    High     │  eCO2 High Byte                               │
│    6    │    Low      │  eCO2 Low Byte → PPM                          │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    7    │    High     │  Temperature High Byte                        │
│    8    │    Low      │  Temperature Low Byte → 0.1°C units *         │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│    9    │    High     │  Humidity High Byte                           │
│   10    │    Low      │  Humidity Low Byte → 0.1%RH units             │
├─────────┼─────────────┼───────────────────────────────────────────────┤
│   11    │  Checksum   │  (~sum(bytes 0-10)) + 1                       │
└─────────┴─────────────┴───────────────────────────────────────────────┘

* Temperature uses special encoding for negative values (see below)
```

## Value Calculation

### Standard 16-bit Values (VOC, Formaldehyde, eCO2, Humidity)

```
Value = (High_Byte << 8) | Low_Byte
      = (High_Byte × 256) + Low_Byte
```

**Example:** VOC reading of 1234 µg/m³
```
1234 = 0x04D2
Byte 1 (High) = 0x04 = 4
Byte 2 (Low)  = 0xD2 = 210
Verification: 4 × 256 + 210 = 1024 + 210 = 1234 ✓
```

### Temperature (with Negative Value Support)

#### Positive Temperatures
```
Raw_Value = (High_Byte << 8) | Low_Byte
If MSB (bit 15) == 0:
    Temperature_Units = Raw_Value
    Temperature_°C = Temperature_Units × 0.1
```

**Example:** 31.7°C
```
Temperature_Units = 317
Raw_Value = 0x013D = 317
Byte 7 = 0x01 = 1
Byte 8 = 0x3D = 61
Temperature = 317 × 0.1 = 31.7°C ✓
```

#### Negative Temperatures
```
Raw_Value = (High_Byte << 8) | Low_Byte
If MSB (bit 15) == 1:
    Magnitude = 0xFFFF - Raw_Value
    Temperature_Units = -Magnitude
    Temperature_°C = Temperature_Units × 0.1
```

**Example 1:** -1.0°C
```
Temperature_Units = -10
Magnitude = 10
Raw_Value = 0xFFFF - 10 = 0xFFF5
Byte 7 = 0xFF = 255
Byte 8 = 0xF5 = 245
Decoding: 0xFFFF - 0xFFF5 = 10, negate = -10
Temperature = -10 × 0.1 = -1.0°C ✓
```

**Example 2:** -10.0°C
```
Temperature_Units = -100
Magnitude = 100 = 0x0064
Raw_Value = 0xFFFF - 100 = 0xFF9B
Byte 7 = 0xFF = 255
Byte 8 = 0x9B = 155
Decoding: 0xFFFF - 0xFF9B = 0x0064 = 100, negate = -100
Temperature = -100 × 0.1 = -10.0°C ✓
```

### Humidity

```
Raw_Value = (High_Byte << 8) | Low_Byte
Humidity_% = Raw_Value × 0.1
```

**Example:** 65.5%RH
```
Humidity_Units = 655
Raw_Value = 0x028F = 655
Byte 9  = 0x02 = 2
Byte 10 = 0x8F = 143
Humidity = 655 × 0.1 = 65.5% ✓
```

## Checksum Calculation

The checksum is a two's complement checksum of bytes 0-10:

```
sum = byte[0] + byte[1] + byte[2] + ... + byte[10]
checksum = (~sum) + 1
```

This is equivalent to: `checksum = -sum` in two's complement arithmetic.

**Example Packet:**
```
Bytes: 2C 00 64 00 32 00 C8 01 3D 02 8F ??

Calculation:
sum = 0x2C + 0x00 + 0x64 + 0x00 + 0x32 + 0x00 + 0xC8 + 0x01 + 0x3D + 0x02 + 0x8F
    = 44 + 0 + 100 + 0 + 50 + 0 + 200 + 1 + 61 + 2 + 143
    = 601 = 0x0259

checksum = ~0x59 + 1  (using only low byte in 8-bit arithmetic)
         = 0xA6 + 1
         = 0xA7

Full packet: 2C 00 64 00 32 00 C8 01 3D 02 8F A7
```

This packet represents:
- VOC: 100 µg/m³
- Formaldehyde: 50 µg/m³
- eCO2: 200 PPM
- Temperature: 31.7°C
- Humidity: 65.5%RH

## Packet Synchronization

The component uses the header byte (0x2C) to synchronize packet boundaries:

1. Wait for header byte 0x2C
2. Read the following 11 bytes
3. Validate checksum
4. Parse data if checksum is valid
5. Discard packet if checksum fails
6. Repeat

This ensures reliable data parsing even if bytes are dropped or the connection is established mid-stream.
