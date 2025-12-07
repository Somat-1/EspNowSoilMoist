#!/bin/bash
# Quick script to get MAC address from new ESP32-C3 controller

echo "=== ESP32-C3 MAC Address Getter ==="
echo ""
echo "1. Connect your NEW 'A&E' controller via USB"
echo "2. Press ENTER when ready..."
read

echo ""
echo "Detecting USB port..."
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -n 1)

if [ -z "$PORT" ]; then
    echo "ERROR: No USB device found!"
    echo "Please check connection and try again."
    exit 1
fi

echo "Found device: $PORT"
echo ""

cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"

echo "Backing up main.cpp..."
mv src/main.cpp src/main.cpp.bak 2>/dev/null

echo "Copying MAC address getter..."
cp src/get_mac_address.cpp.example src/main.cpp

echo ""
echo "Uploading MAC getter to $PORT..."
~/.platformio/penv/bin/platformio run --target upload --upload-port "$PORT"

echo ""
echo "Opening serial monitor in 3 seconds..."
echo "Watch for the MAC address output!"
sleep 3

~/.platformio/penv/bin/platformio device monitor --port "$PORT"

echo ""
echo "Restoring main.cpp..."
mv src/main.cpp.bak src/main.cpp

echo ""
echo "=== Done! ==="
echo "Did you copy the MAC address? (format: 0xXX, 0xXX, ...)"
echo "If not, run this script again!"
