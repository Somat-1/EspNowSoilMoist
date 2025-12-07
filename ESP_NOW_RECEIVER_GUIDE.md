# ESP-NOW Receiver Setup Guide

## 📡 How ESP-NOW Works

ESP-NOW is a **peer-to-peer wireless communication protocol** developed by Espressif for ESP32 devices. It allows:
- Direct communication between ESP32 devices **without WiFi router**
- Very low latency (< 10ms)
- Low power consumption
- Communication range: 200-300 meters in open space

### Communication Flow:
```
┌─────────────────────────────┐         ┌─────────────────────────────┐
│  Soil Moisture Sensor       │         │  Hub/Receiver ESP32         │
│  (Sender)                   │         │                             │
│  MAC: 80:F1:B2:50:29:74    │────────▶│  MAC: ??.??.??.??.??.??   │
│                             │ ESP-NOW │                             │
│  1. Wake from sleep         │         │  1. Always listening        │
│  2. Read sensors            │         │  2. Receive data            │
│  3. Send data packet        │         │  3. Display/Store data      │
│  4. Go back to sleep        │         │  4. Wait for next packet    │
└─────────────────────────────┘         └─────────────────────────────┘
```

## 🎯 Data Structure Being Sent

Your sensor sends this data every 5 seconds (or 15 minutes in production):

```cpp
struct sensor_data_t {
  float batteryVoltage;      // Battery voltage in Volts (e.g., 4.15)
  int batteryPercent;        // Battery percentage 0-100
  int soilMoisture;          // Soil moisture percentage 0-100
  unsigned long timestamp;   // Milliseconds since sensor boot
};
```

**Size:** 16 bytes (4 + 4 + 4 + 4)

## 🔧 Step-by-Step Receiver Setup

### Step 1: Get Your Hub's MAC Address

1. Connect your hub ESP32 to computer
2. Upload the MAC address getter code:

```bash
cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
# Copy the example file to test
cp src/get_mac_address.cpp.example /tmp/test_mac.cpp
```

Or manually create a temporary sketch:
```cpp
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  
  Serial.println("\n\n=== ESP32 MAC Address ===");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("========================\n");
}

void loop() {
  delay(1000);
}
```

3. Open serial monitor and note the MAC address (e.g., `AA:BB:CC:DD:EE:FF`)

### Step 2: Update Sender with Hub's MAC Address

Edit `src/main.cpp` line ~22:

**BEFORE:**
```cpp
uint8_t hubMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

**AFTER (use YOUR hub's MAC):**
```cpp
uint8_t hubMacAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
```

### Step 3: Upload Receiver Code to Hub

Use the `src/hub_receiver.cpp.example` file. Here's what it does:

```cpp
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Same data structure as sender
typedef struct sensor_data_t {
  float batteryVoltage;
  int batteryPercent;
  int soilMoisture;
  unsigned long timestamp;
} sensor_data_t;

sensor_data_t receivedData;

// Callback when data is received
void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  memcpy(&receivedData, data, sizeof(receivedData));
  
  Serial.println("\n=== Data Received ===");
  Serial.printf("From MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac_addr[0], mac_addr[1], mac_addr[2],
                mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("Battery: %.2f V (%d%%)\n", 
                receivedData.batteryVoltage, 
                receivedData.batteryPercent);
  Serial.printf("Soil Moisture: %d%%\n", receivedData.soilMoisture);
  Serial.printf("Timestamp: %lu ms\n", receivedData.timestamp);
  Serial.println("====================\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP-NOW Hub Receiver ===");
  
  WiFi.mode(WIFI_STA);
  
  Serial.print("Hub MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  Serial.println("ESP-NOW initialized");
  esp_now_register_recv_cb(onDataReceived);
  
  Serial.println("Waiting for sensor data...\n");
}

void loop() {
  delay(100);
}
```

### Step 4: Test the Communication

1. **Upload receiver code to hub** → Open serial monitor on hub
2. **Press RESET on sensor** → Sensor sends data
3. **Check hub serial monitor** → You should see:

```
=== ESP-NOW Hub Receiver ===
Hub MAC Address: AA:BB:CC:DD:EE:FF
ESP-NOW initialized
Waiting for sensor data...

=== Data Received ===
From MAC: 80:F1:B2:50:29:74
Battery: 4.15 V (95%)
Soil Moisture: 45%
Timestamp: 1234 ms
====================
```

## 🔍 Troubleshooting ESP-NOW

### Problem: Hub doesn't receive data

**Check 1: MAC Address Configuration**
- Verify hub's MAC address is correctly entered in sensor code
- MAC format: `{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}`

**Check 2: Both devices on same WiFi channel**
- ESP-NOW requires same WiFi channel
- By default, both should use channel 0 (auto)

**Check 3: Distance**
- Keep devices within 20 meters for initial testing
- Remove obstacles between devices

**Check 4: Power**
- Ensure sensor has enough battery voltage (> 3.3V)
- Check USB power on hub

### Problem: Intermittent reception

- Check WiFi interference
- Try different WiFi channels
- Check antenna orientation
- Verify power supply stability

### Problem: Wrong data values

- Verify data structure matches on both sender and receiver
- Check endianness (should be same on ESP32)
- Print raw bytes to debug

## 📊 What You'll See

### On Sensor (Sender) - Every 5 seconds:
```
=== ESP32-C3 Soil Moisture Sensor ===
Battery: 4.15 V (95%)
Raw ADC average: 2500
Soil Moisture: 51%
Sending data via ESP-NOW...
Send initiated successfully
Send Status: Success
Waiting 5 seconds before next reading...
```

### On Hub (Receiver) - When data arrives:
```
=== Data Received ===
From MAC: 80:F1:B2:50:29:74
Battery: 4.15 V (95%)
Soil Moisture: 51%
Timestamp: 1234 ms
====================
```

## 🚀 Production Setup (15-minute intervals)

Once testing is complete, change the sensor sleep time:

In `src/main.cpp`:
```cpp
#define SLEEP_DURATION_US (15 * 60 * 1000000ULL)  // 15 minutes
```

And uncomment deep sleep:
```cpp
esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
esp_deep_sleep_start();
```

## 📝 Key Points

1. **No WiFi router needed** - ESP-NOW is peer-to-peer
2. **Hub must be powered** - It needs to always listen
3. **Sensor MAC**: `80:F1:B2:50:29:74` (your current sensor)
4. **Data structure must match** exactly on both sender and receiver
5. **One-way communication** - Sender → Receiver (no acknowledgment needed for your use case)

## 🔄 Adding Multiple Sensors

To receive from multiple sensors:

1. Each sensor needs the **same hub MAC address**
2. Hub can identify sensors by their **unique MAC addresses**
3. Hub automatically accepts from all senders (no need to register peers on receiver side)

Example hub code modification:
```cpp
void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // Identify which sensor sent data
  if (mac_addr[0] == 0x80 && mac_addr[1] == 0xF1) {
    Serial.println("Data from Sensor #1 (Soil Moisture)");
  }
  // Add more sensors...
}
```

## 📦 Files You Need

1. **`src/get_mac_address.cpp.example`** - Get MAC of hub
2. **`src/hub_receiver.cpp.example`** - Full receiver code
3. **`src/main.cpp`** - Current sender (update hub MAC on line 22)

## Next Steps

1. ✅ Get hub MAC address
2. ✅ Update sender with hub MAC
3. ✅ Upload receiver to hub
4. ✅ Upload sender to sensor
5. ✅ Test communication
6. ✅ Switch to 15-minute intervals for production
