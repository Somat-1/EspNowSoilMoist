#!/bin/bash
# Quick command to re-upload firmware to ESP32-C3 Soil Moisture Sensor

cd "/Users/tomasvalentinas/Documents/PlatformIO/Projects/Soil Moisture"

echo "🔨 Building firmware..."
~/.platformio/penv/bin/platformio run

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "📤 Uploading to ESP32-C3..."
    echo "⚠️  Make sure serial monitor is closed!"
    
    # Kill any serial monitors
    pkill -f "device monitor" 2>/dev/null
    sleep 1
    
    ~/.platformio/penv/bin/platformio run --target upload
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Upload successful!"
        echo ""
        echo "🔍 Opening serial monitor (press RESET on device to see output)..."
        echo "   Press Ctrl+C to exit monitor"
        sleep 2
        ~/.platformio/penv/bin/platformio device monitor
    else
        echo "❌ Upload failed!"
    fi
else
    echo "❌ Build failed!"
fi
