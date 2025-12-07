# ESP-NOW Soil Moisture Sensor - Complete Guide

## 🔌 Your Hardware Setup (Corrected)

```
ESP32-C3 Connections:
├── GPIO0: Battery voltage sense (via 100kΩ + 100kΩ divider)
├── GPIO1: Soil moisture sensor AOUT (analog signal)
├── 5V:    Power from TP4056 OUT+
└── GND:   Common ground

No GPIO2 used - sensor is always powered!
```

## 📊 What the Sensor Does

Every 30 seconds (for testing), the ESP32-C3:
1. ✅ Wakes up from deep sleep
2. ✅ Reads battery voltage from GPIO0
3. ✅ Reads soil moisture from GPIO1 (10 samples averaged)
4. ✅ Sends data via ESP-NOW to hub
5. ✅ Goes back to deep sleep

## 🚀 How to Upload & Test

### Step 1: Connect ESP32-C3 via USB

Press the **BOOT** button while plugging in USB if it doesn't show up automatically.

### Step 2: Upload Firmware

```bash
cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
~/.platformio/penv/bin/platformio run --target upload
```

Or simply run:
```bash
./upload.sh
```

### Step 3: Monitor Output

After upload completes, immediately open serial monitor:
```bash
~/.platformio/penv/bin/platformio device monitor
```

You'll see:
```
=== ESP32-C3 Soil Moisture Sensor ===
Battery: 4.15 V (95%)
Raw ADC average: 2500
Soil Moisture: 51%
Sending data via ESP-NOW...
ESP-NOW initialized
Peer added successfully
Send initiated successfully
Send Status: Success (or Fail if no hub)
Going to sleep for 30 seconds...
```

### Step 4: Device Disconnects

⚠️ **Important**: After going to sleep, the device disconnects from USB. This is normal!

To see the next reading:
- Press **RESET** button on ESP32-C3
- Or wait 30 seconds for it to wake up automatically

## 📡 ESP-NOW Communication Setup

### Architecture

```
┌─────────────────────┐         ┌─────────────────────┐
│  Soil Sensor        │         │  Hub Receiver       │
│  (This Device)      │  ESP-NOW │  (Another ESP32)    │
│  MAC: 80:F1:B2:..   │────────>│  MAC: ??:??:??:..   │
│                     │         │                     │
│  Sends:             │         │  Receives:          │
│  - Battery V & %    │         │  - Displays data    │
│  - Soil moisture %  │         │  - Logs to serial   │
│  - Timestamp        │         │  - Can forward to   │
│                     │         │    WiFi/MQTT/etc    │
└─────────────────────┘         └─────────────────────┘
```

### Data Structure Being Sent

```cpp
struct sensor_data_t {
  float batteryVoltage;      // e.g., 4.15 V
  int batteryPercent;        // e.g., 95%
  int soilMoisture;          // e.g., 45%
  unsigned long timestamp;   // milliseconds since boot
};
```

## 🎯 Setting Up ESP-NOW Communication

### STEP 1: Get Your Hub's MAC Address

**On your hub ESP32**, upload this code (it's in `src/get_mac_address.cpp.example`):

```cpp
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  
  Serial.println("\n=== ESP32 MAC Address ===");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("========================");
}

void loop() {
  delay(5000);
}
```

**You'll see output like:**
```
=== ESP32 MAC Address ===
MAC Address: AA:BB:CC:DD:EE:FF
========================
```

### STEP 2: Update Sender with Hub's MAC

Edit `main.cpp` line 22:

**Before:**
```cpp
uint8_t hubMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

**After (using your hub's actual MAC):**
```cpp
// Example: if hub MAC is AA:BB:CC:DD:EE:FF
uint8_t hubMacAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
```

### STEP 3: Upload Receiver Code to Hub

Use this code on your hub (it's in `src/hub_receiver.cpp.example`):

```cpp
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Data structure (must match sender)
typedef struct sensor_data_t {
  float batteryVoltage;
  int batteryPercent;
  int soilMoisture;
  unsigned long timestamp;
} sensor_data_t;

sensor_data_t receivedData;

// Callback when data is received
void onDataReceived(const esp_now_recv_info_t *mac_info, const uint8_t *data, int len) {
  memcpy(&receivedData, data, sizeof(receivedData));
  
  Serial.println("\n=== Data Received ===");
  Serial.printf("From: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac_info->src_addr[0], mac_info->src_addr[1], mac_info->src_addr[2],
    mac_info->src_addr[3], mac_info->src_addr[4], mac_info->src_addr[5]);
  
  Serial.printf("Battery: %.2f V (%d%%)\n", 
    receivedData.batteryVoltage, receivedData.batteryPercent);
  Serial.printf("Soil Moisture: %d%%\n", receivedData.soilMoisture);
  Serial.printf("Timestamp: %lu ms\n", receivedData.timestamp);
  Serial.println("====================\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP-NOW Hub Receiver ===");
  
  // Show MAC address
  WiFi.mode(WIFI_STA);
  Serial.print("Hub MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  Serial.println("ESP-NOW initialized successfully");
  
  // Register receive callback
  esp_now_register_recv_cb(onDataReceived);
  
  Serial.println("Waiting for data...\n");
}

void loop() {
  delay(100);
}
```

### STEP 4: Re-upload Sender with Correct MAC

After updating the hub MAC address in `main.cpp`:
```bash
./upload.sh
```

### STEP 5: Test Communication

1. **Start hub receiver** - should show "Waiting for data..."
2. **Reset sender device** - it will wake up and send data
3. **Hub displays:** Battery, soil moisture, timestamp

## 🔍 Example Output

### On Sender (Soil Sensor):
```
=== ESP32-C3 Soil Moisture Sensor ===
Battery: 4.15 V (95%)
Raw ADC average: 2500
Soil Moisture: 51%
Sending data via ESP-NOW...
ESP-NOW initialized
Peer added successfully
Send initiated successfully
Send Status: Success
Data sent successfully!
Going to sleep for 30 seconds...
```

### On Hub (Receiver):
```
=== ESP-NOW Hub Receiver ===
Hub MAC Address: AA:BB:CC:DD:EE:FF
ESP-NOW initialized successfully
Waiting for data...

=== Data Received ===
From: 80:F1:B2:50:29:74
Battery: 4.15 V (95%)
Soil Moisture: 51%
Timestamp: 2345 ms
====================
```

## 🛠️ Troubleshooting

### "Send Status: Fail"
- ✅ Hub MAC address is correct in `main.cpp`
- ✅ Hub receiver is running
- ✅ Both devices are within range (~50m)

### No data on hub
- ✅ Hub MAC address matches what you got from Step 1
- ✅ Data structure is identical on both devices
- ✅ Hub has ESP-NOW initialized and callback registered

### Battery reading always 0V
- ✅ Check voltage divider: R1=100kΩ, R2=100kΩ
- ✅ Verify GPIO0 connection to divider midpoint
- ✅ Measure actual voltage with multimeter

### Soil reading stuck at 0% or 100%
- ✅ Recalibrate SOIL_DRY_VALUE (in air)
- ✅ Recalibrate SOIL_WET_VALUE (in water)
- ✅ Check GPIO1 connection to sensor AOUT
- ✅ Ensure sensor is powered (VCC connected)

## 📝 Quick Commands

```bash
# Build
~/.platformio/penv/bin/platformio run

# Upload
~/.platformio/penv/bin/platformio run --target upload

# Monitor
~/.platformio/penv/bin/platformio device monitor

# Or use helper script
./upload.sh
```

## 🔋 Battery Life

With 1100 mAh LiPo:
- Active: ~2 seconds every 30 seconds (testing)
- Sleep: 10 µA
- Active: 80 mA

**For 15-minute intervals** (production):
Change line 16 in `main.cpp`:
```cpp
#define SLEEP_DURATION_US (15 * 60 * 1000000ULL)  // 15 minutes
```

Expected battery life: **Several months**

## 📌 Your Device Info

- **Sensor MAC:** `80:F1:B2:50:29:74`
- **Hub MAC:** `(Add after Step 1)`
- **Calibration:**
  - Dry (air): 3738 ADC → 0%
  - Wet (water): 1324 ADC → 100%
