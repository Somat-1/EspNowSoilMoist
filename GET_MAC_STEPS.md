# Steps to Add New "A&E" Sensor

## Step 1: Get the MAC Address of the New Controller

1. **Temporarily rename main.cpp:**
   ```bash
   cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
   mv src/main.cpp src/main.cpp.bak
   ```

2. **Copy the MAC address getter to main.cpp:**
   ```bash
   cp src/get_mac_address.cpp.example src/main.cpp
   ```

3. **Upload to the new controller:**
   - Connect the new "A&E" controller via USB
   - Find the port (e.g., `/dev/cu.usbmodem3102` or similar)
   - Upload and monitor:
   ```bash
   pio run --target upload --upload-port /dev/cu.usbmodemXXXX && pio device monitor
   ```

4. **Copy the MAC address:**
   - You'll see output like:
   ```
   Your MAC Address:
   80:F1:B2:50:29:XX
   
   Copy this to your sender code:
   uint8_t hubMacAddress[] = {0x80, 0xF1, 0xB2, 0x50, 0x29, 0xXX};
   ```
   - **SAVE THIS MAC ADDRESS!** You'll need it for Step 3.

5. **Restore the original main.cpp:**
   ```bash
   mv src/main.cpp.bak src/main.cpp
   ```

---

## Step 2: Update the Sensor Code with "A&E" Name

The sensor code in `Soil Moisture/src/main.cpp` needs no changes - it already:
- ✅ Sends to hub MAC: `{0x80, 0xF1, 0xB2, 0x50, 0x29, 0x70}` (TramReader)
- ✅ Takes 3 measurements every 6 hours
- ✅ Averages battery and soil moisture
- ✅ Uses BAT_CAL = 0.915
- ✅ LED stays off

Just upload it to the new "A&E" controller:
```bash
cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
pio run --target upload --upload-port /dev/cu.usbmodemXXXX
```

---

## Step 3: Update TramReader to Display "A&E" Data

You need to:
1. **Identify which sensor MAC is which** in the receiver
2. **Add display logic for "A&E"** in the third section (currently showing "---")

### 3a. Update `espnow_receiver.h` to handle multiple sensors

The receiver should store data from multiple sensors based on their MAC addresses.

### 3b. Update `disp.cpp` to show "A&E" data

Currently the display shows:
- Section 1: "Olga" (MAC: 80:F1:B2:50:29:74)
- Section 2: "Rose" (placeholder with "--")
- Section 3: "---" (placeholder)

You need to map:
- "Olga" → MAC: `80:F1:B2:50:29:74` (existing)
- "Rose" → MAC: `XX:XX:XX:XX:XX:XX` (future, or swap with A&E)
- "A&E" → MAC: `[NEW MAC FROM STEP 1]`

---

## Quick Commands Summary

```bash
# Step 1: Get MAC address of new controller
cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"
mv src/main.cpp src/main.cpp.bak
cp src/get_mac_address.cpp.example src/main.cpp
pio run --target upload --upload-port /dev/cu.usbmodemXXXX && pio device monitor
# COPY THE MAC ADDRESS FROM OUTPUT!
mv src/main.cpp.bak src/main.cpp

# Step 2: Upload sensor code to "A&E" controller
pio run --target upload --upload-port /dev/cu.usbmodemXXXX

# Step 3: Tell me the MAC address and I'll update TramReader display
```

---

## What I Need From You

After completing Step 1, please provide:
1. The MAC address of the new "A&E" controller
2. Which section you want it displayed in:
   - Option A: Replace "Rose" (Section 2) with "A&E"
   - Option B: Replace "---" (Section 3) with "A&E"
   - Option C: Keep "Rose" and add "A&E" to Section 3

Then I'll update the TramReader code to display the "A&E" sensor data!
