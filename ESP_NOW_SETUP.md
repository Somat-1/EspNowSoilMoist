# ESP-NOW Soil Moisture Sensor - Setup Guide

## ✅ Firmware Upload Status
**Successfully uploaded to ESP32-C3!**
- Date: December 7, 2025
- Device MAC Address: **80:F1:B2:50:29:74**
- Build Size: 736KB Flash, 37KB RAM

## 📋 Current Configuration

### Hardware Pinout (Correctly Configured)
- **GPIO0** → Battery voltage divider (R1=100kΩ, R2=100kΩ)
- **GPIO1** → Soil sensor analog output (AO)
- **GPIO8** → Onboard LED (status indicator)
- **5V** → from TP4056 OUT+
- **GND** → Common ground

**Note:** Sensor is always powered (VCC connected to 5V), no power switching needed.

### Sensor Calibration
- **Dry (Air)**: 3738 → 0% moisture
- **Wet (Submerged)**: 1324 → 100% moisture

### Power Management
- **Sleep Duration**: Currently 5 seconds for debugging (change to 15 minutes for production)
- **Deep Sleep**: Currently disabled for debugging (will enable for production)
- **LED Blink**: Blinks 3 times on startup to indicate successful boot

## 🔧 Next Steps - Hub Receiver Setup

### IMPORTANT: Update Hub MAC Address
You need to replace the placeholder MAC address in your sender code:

**Current placeholder in `main.cpp`:**
```cpp
uint8_t hubMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

### To Set Up the Hub Receiver:

1. **Get Hub's MAC Address:**
   - Upload `src/get_mac_address.cpp.example` to your hub ESP32
   - Rename it to `main.cpp` temporarily
   - Upload and check serial monitor for MAC address
   - Example output: `MAC Address: AA:BB:CC:DD:EE:FF`

2. **Update Sender with Hub's MAC:**
   ```cpp
   // Replace in main.cpp line 22
   uint8_t hubMacAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
   ```

3. **Upload Hub Receiver Code:**
   - Use `src/hub_receiver.cpp.example` as your hub's main.cpp
   - This will receive and display sensor data

4. **Re-upload Sender:**
   - After updating the MAC address
   - Upload back to this ESP32-C3 sensor

## 📊 Data Being Sent

The sensor sends this structure via ESP-NOW:
```cpp
struct sensor_data_t {
  float batteryVoltage;      // e.g., 4.15 V
  int batteryPercent;        // e.g., 95%
  int soilMoisture;          // e.g., 45%
  unsigned long timestamp;   // milliseconds since boot
}
```

## 🔍 Testing the Sensor

### To See Current Output:
1. Press RESET button on ESP32-C3
2. Open serial monitor immediately:
   ```bash
   ~/.platformio/penv/bin/platformio device monitor
   ```
3. You should see:
   ```
   === ESP32-C3 Soil Moisture Sensor ===
   Battery: 4.15 V (95%)
   Raw ADC average: 2500
   Soil Moisture: 51%
   Sending data via ESP-NOW...
   Send initiated successfully
   Send Status: Success/Fail
   Going to sleep for 15 minutes...
   ```

### Current Behavior:
- Device wakes up every 15 minutes
- Takes sensor readings
- Sends data via ESP-NOW
- Goes back to deep sleep
- **The device will disconnect from USB after sleeping**

## 📁 Example Files Created

1. **`get_mac_address.cpp.example`** - Get MAC address of any ESP32
2. **`hub_receiver.cpp.example`** - Full ESP-NOW receiver for hub

## 🔋 Battery Life Estimation

With 1100 mAh battery:
- Active time: ~2 seconds every 15 minutes
- Deep sleep current: ~10 µA
- Active current: ~80 mA
- **Expected battery life: Several months**

## 🛠️ Troubleshooting

### If sensor doesn't send data:
1. Check hub MAC address is correct
2. Ensure both devices are on same WiFi channel
3. Check serial monitor for error messages

### If battery reading is wrong:
1. Verify voltage divider (R1=100kΩ, R2=100kΩ)
2. Check GPIO0 connection
3. Measure actual battery voltage with multimeter

### If soil readings are wrong:
1. Recalibrate SOIL_DRY_VALUE and SOIL_WET_VALUE
2. Check GPIO1 and GPIO2 connections
3. Ensure sensor is powered (GPIO2 HIGH during reading)

## 📝 Your Sensor MAC Address
**80:F1:B2:50:29:74**

Save this! You'll need it if you want to identify this specific sensor on the hub.
