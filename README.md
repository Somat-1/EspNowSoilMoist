# ESP32-C3 Soil Moisture Sensor with ESP-NOW

## Project Overview
Battery-powered soil moisture sensor using ESP32-C3 that sends data via ESP-NOW to a hub controller.

## Hardware Setup

### Battery & Power
- **Battery**: 1-cell LiPo, 3.7V, 1100mAh
- **Charger**: TP4056
- **Wiring**:
  - LiPo + → TP4056 BAT+
  - LiPo − → TP4056 BAT−
  - TP4056 OUT+ → ESP32-C3 5V
  - TP4056 OUT− → ESP32-C3 GND

### Battery Voltage Divider
- R1 = 100kΩ, R2 = 100kΩ
- BAT+ → R1 → GPIO0 → R2 → GND
- ESP ADC reads ~½ of battery voltage

### Capacitive Soil Moisture Sensor
- **Power Control**: GPIO2 (HIGH = on, LOW = off)
- **Signal**: GPIO1 (ADC input)
- **Calibration**:
  - Air/Dry: 3738 → 0% moisture
  - Fully submerged: 1324 → 100% moisture

### Pin Assignments
- GPIO0: Battery voltage measurement (ADC)
- GPIO1: Soil moisture sensor signal (ADC)
- GPIO2: Soil sensor power control (output)

## Software Features

### Power Management
- Deep sleep between readings (15 minutes default)
- Sensor powered only during measurement (~300ms)
- ESP-NOW for low-power wireless communication
- Estimated battery life: several months

### Measurements
- Battery voltage and percentage
- Soil moisture percentage (0-100%)
- Averaged readings for accuracy

### ESP-NOW Communication
- Wireless data transmission to hub
- No WiFi connection needed (peer-to-peer)
- Low power consumption
- Long range mode enabled

## Setup Instructions

### 1. Get Hub MAC Address
First, you need the MAC address of your hub/receiver ESP32:

**Option A**: If you have a hub already running:
- Upload `get_mac_address.cpp.example` to your hub ESP32
- Rename it to `main.cpp` temporarily
- Build and upload
- Open Serial Monitor to see the MAC address

**Option B**: Use the hub receiver code:
- The hub receiver code prints its MAC address on startup

### 2. Update Sender MAC Address
In `src/main.cpp`, update this line with your hub's MAC address:
```cpp
uint8_t hubMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

Replace with the actual MAC address, for example:
```cpp
uint8_t hubMacAddress[] = {0xA4, 0xCF, 0x12, 0x34, 0x56, 0x78};
```

### 3. Build and Upload Sender
```bash
cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
pio run --target upload
pio device monitor
```

### 4. Setup Hub/Receiver
Create a new PlatformIO project for your hub or use existing:
- Copy contents from `hub_receiver.cpp.example` to the hub's `main.cpp`
- Build and upload to hub ESP32
- Hub will print its MAC address and wait for data

## Configuration

### Adjust Sleep Duration
In `main.cpp`, modify:
```cpp
#define SLEEP_DURATION_US (15 * 60 * 1000000ULL)  // 15 minutes
```

### Calibrate Soil Sensor
1. Measure ADC value in air (dry):
   ```cpp
   #define SOIL_DRY_VALUE 3738
   ```

2. Measure ADC value in water (wet):
   ```cpp
   #define SOIL_WET_VALUE 1324
   ```

### Adjust Sample Count
```cpp
#define NUM_SAMPLES 10  // Number of readings to average
```

## Monitoring

### View Sensor Output
```bash
pio device monitor
```

You should see:
```
=== ESP32-C3 Soil Moisture Sensor ===
Battery: 3.85 V (71%)
Raw ADC average: 2450
Soil Moisture: 53%
Sending data via ESP-NOW...
Send initiated successfully
Data sent successfully!
Going to sleep for 15 minutes...
```

### View Hub Output
The hub will display received data:
```
=== Data Received ===
From MAC: XX:XX:XX:XX:XX:XX
Battery: 3.85 V (71%)
Soil Moisture: 53%
Timestamp: 1234 ms
====================
```

## Troubleshooting

### Data Not Sending
- Verify hub MAC address is correct
- Check both devices are powered on
- Ensure hub is within range (~100m outdoors, less indoors)
- Verify both devices use same WiFi channel

### Battery Reading Incorrect
- Check voltage divider resistors (should be 100kΩ each)
- Verify wiring to GPIO0
- Adjust voltage calculation if needed

### Soil Moisture Reading Incorrect
- Recalibrate dry/wet values
- Check sensor is powered from GPIO2
- Verify sensor signal connected to GPIO1
- Increase stabilization delay if needed

## Future Enhancements
- [ ] Add multiple sensors support
- [ ] Implement data logging on hub
- [ ] Add MQTT/cloud integration on hub
- [ ] Display on OLED/LCD
- [ ] Alert notifications for low moisture
- [ ] Web interface for data visualization

## Power Consumption Estimates
- Active time: ~2-3 seconds per reading
- Sleep current: ~10-20 µA
- Active current: ~80-100 mA
- With 1100mAh battery and 15-min intervals: 3-6 months battery life
