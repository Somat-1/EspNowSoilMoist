#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ========== PIN DEFINITIONS ==========
#define BATTERY_ADC_PIN   0   // GPIO0 - Battery voltage divider (R1=100k, R2=100k)
#define SENSOR_SIGNAL_PIN 1   // GPIO1 - Capacitive soil sensor analog output (AOUT)
#define LED_PIN           8   // GPIO8 - Onboard LED

// ========== CALIBRATION VALUES ==========
#define SOIL_DRY_VALUE    3738  // ADC value in air (0% moisture)
#define SOIL_WET_VALUE    1324  // ADC value fully submerged (100% moisture)

// ========== POWER MANAGEMENT ==========
#define SLEEP_DURATION_US (5 * 1000000ULL)  // 5 seconds for debugging
#define NUM_SAMPLES       10    // Number of samples to average

// ========== ESP-NOW CONFIGURATION ==========
// MAC address of the receiver (hub) - REPLACE WITH YOUR HUB'S MAC ADDRESS
uint8_t hubMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Data structure to send
typedef struct sensor_data_t {
  float batteryVoltage;
  int batteryPercent;
  int soilMoisture;
  unsigned long timestamp;
} sensor_data_t;

sensor_data_t sensorData;

// ESP-NOW send status
bool dataSent = false;

// ========== FUNCTION DECLARATIONS ==========
void blinkLED(int times, int delayMs);
float readBatteryVoltage();
int calculateBatteryPercent(float voltage);
int readSoilMoisture();
void initESPNow();
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Configure LED and blink on startup
  pinMode(LED_PIN, OUTPUT);
  blinkLED(3, 150);  // Blink 3 times, 150ms each
  
  Serial.println("\n\n=== ESP32-C3 Soil Moisture Sensor ===");
  
  // Configure ADC pins
  pinMode(BATTERY_ADC_PIN, INPUT);
  pinMode(SENSOR_SIGNAL_PIN, INPUT);
  
  // Read battery voltage
  sensorData.batteryVoltage = readBatteryVoltage();
  sensorData.batteryPercent = calculateBatteryPercent(sensorData.batteryVoltage);
  
  Serial.printf("Battery: %.2f V (%d%%)\n", sensorData.batteryVoltage, sensorData.batteryPercent);
  
  // Read soil moisture
  sensorData.soilMoisture = readSoilMoisture();
  Serial.printf("Soil Moisture: %d%%\n", sensorData.soilMoisture);
  
  // Timestamp (milliseconds since boot)
  sensorData.timestamp = millis();
  
  // Initialize WiFi and ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Set WiFi to long range mode for better connectivity
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
  
  initESPNow();
  
  // Send data
  Serial.println("Sending data via ESP-NOW...");
  esp_err_t result = esp_now_send(hubMacAddress, (uint8_t *) &sensorData, sizeof(sensorData));
  
  if (result == ESP_OK) {
    Serial.println("Send initiated successfully");
  } else {
    Serial.println("Error sending data");
  }
  
  // Wait for send confirmation (with timeout)
  unsigned long startTime = millis();
  while (!dataSent && (millis() - startTime < 2000)) {
    delay(10);
  }
  
  if (dataSent) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Send timeout or failed");
  }
  
  // Clean up
  esp_now_deinit();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
  // DEBUG MODE: Instead of deep sleep, just delay and repeat
  Serial.printf("Waiting %d seconds before next reading...\n", (int)(SLEEP_DURATION_US / 1000000));
  Serial.println("========================================\n");
  
  delay(SLEEP_DURATION_US / 1000);  // Convert microseconds to milliseconds
  
  // Restart the ESP to simulate wake from deep sleep
  ESP.restart();
}

void loop() {
  // Never reached due to restart
}

// ========== BATTERY MEASUREMENT ==========
float readBatteryVoltage() {
  // Voltage divider: R1=100k, R2=100k (50% divider)
  // ADC reads 0-3.3V, representing 0-6.6V actual battery voltage
  
  int adcValue = analogRead(BATTERY_ADC_PIN);
  
  // ESP32-C3 ADC: 12-bit (0-4095) for 0-3.3V
  // Actual voltage = (adcValue / 4095) * 3.3 * 2 (because of 50% divider)
  float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
  
  return voltage;
}

int calculateBatteryPercent(float voltage) {
  // LiPo voltage range: 3.0V (empty) to 4.2V (full)
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.0) return 0;
  
  // Linear mapping
  int percent = (int)((voltage - 3.0) / (4.2 - 3.0) * 100);
  return constrain(percent, 0, 100);
}

// ========== SOIL MOISTURE MEASUREMENT ==========
int readSoilMoisture() {
  // Sensor is always powered, just read it
  // Take multiple samples and average
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(SENSOR_SIGNAL_PIN);
    delay(10);
  }
  int avgValue = sum / NUM_SAMPLES;
  
  Serial.printf("Raw ADC average: %d\n", avgValue);
  
  // Map to 0-100% (higher ADC = drier = lower moisture %)
  int moisture = map(avgValue, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  moisture = constrain(moisture, 0, 100);
  
  return moisture;
}

// ========== ESP-NOW FUNCTIONS ==========
void initESPNow() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  Serial.println("ESP-NOW initialized");
  
  // Register send callback
  esp_now_register_send_cb(onDataSent);
  
  // Register peer (hub)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, hubMacAddress, 6);
  peerInfo.channel = 0;  // Use current channel
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("Peer added successfully");
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  dataSent = true;
}

// ========== LED FUNCTIONS ==========
void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}