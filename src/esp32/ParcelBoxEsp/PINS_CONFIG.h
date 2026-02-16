#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// ============================================================================
// ESP32 PIN CONFIGURATION - Smart Parcel Locker
// ============================================================================
// This file centralizes all pin assignments for the ParcelBox system

// ============================================================================
// ESP32 I2C LCD DISPLAY PINS
// ============================================================================
#define LCD_SDA_PIN 21           // I2C Data line (GPIO21)
#define LCD_SCL_PIN 22           // I2C Clock line (GPIO22)
#define LCD_I2C_ADDRESS 0x27     // I2C address for 20x4 LCD
#define LCD_COLS 20              // LCD columns
#define LCD_ROWS 4               // LCD rows

// ============================================================================
// ESP32 UART COMMUNICATION PINS
// ============================================================================
// Serial0 (Hardware UART) - Arduino Uno Communication
#define ARDUINO_RX_PIN 3         // GPIO3 - RX from Arduino (Serial RX1)
#define ARDUINO_TX_PIN 1         // GPIO1 - TX to Arduino (Serial TX1)
#define BAUD_ARDUINO 115200      // Baud rate for Arduino communication

// Serial2 (Hardware UART) - QR Code Scanner
#define QR_SCANNER_RX_PIN 16     // GPIO16 - RX from QR Scanner (Serial2 RX)
#define QR_SCANNER_TX_PIN 17     // GPIO17 - TX to QR Scanner (Serial2 TX)
#define BAUD_QR_SCANNER 9600     // Baud rate for QR Scanner

// ============================================================================
// ARDUINO UNO PIN CONFIGURATION (Reference for dual-controller setup)
// ============================================================================
// Relay Control (Solenoid Locks) - Arduino Digital Pins
#define RELAY_1_PIN 2            // D2 - Relay channel 1 (Parcel door lock)
#define RELAY_2_PIN 3            // D3 - Relay channel 2 (Payment box lock)

// Piezo Buzzer - Arduino Digital Pin
#define BUZZER_PIN 4             // D4 - Piezo buzzer for audio feedback

// Door Sensors (Reed Switches) - Arduino Digital Pins (with pull-up)
#define DOOR_SENSOR_1_PIN 5      // D5 - Parcel door open/close sensor
#define DOOR_SENSOR_2_PIN 6      // D6 - Payment box door sensor

// SIM800L GSM Module - Arduino Software Serial
#define SIM800L_RX_PIN 8         // D8 - RX from SIM800L (SoftwareSerial RX)
#define SIM800L_TX_PIN 9         // D9 - TX to SIM800L (SoftwareSerial TX)
#define BAUD_SIM800L 115200      // Baud rate for SIM800L

// ESP32 Communication - Arduino Hardware UART
#define ESP32_RX_PIN RX1         // RX1 (Serial UART) - RX from ESP32
#define ESP32_TX_PIN TX1         // TX1 (Serial UART) - TX to ESP32
#define BAUD_ESP32 115200        // Baud rate for ESP32 communication

// ============================================================================
// TIMING CONSTANTS (milliseconds)
// ============================================================================
#define LOCK_OPERATION_DELAY 500      // Delay between lock operations
#define BUZZER_SUCCESS_DURATION 200   // Success beep duration (ms)
#define BUZZER_ALERT_DURATION 300     // Alert beep duration (ms)
#define SYSTEM_HEALTH_CHECK_INTERVAL 30000  // System health check every 30s
#define ARDUINO_RESPONSE_TIMEOUT 5000 // Wait for Arduino response (5 seconds)
#define QR_SCAN_DEBOUNCE 500          // QR scan debounce time

// ============================================================================
// COMMUNICATION PROTOCOL DEFINITIONS
// ============================================================================
// Arduino AT-style Commands
#define CMD_LOCK_OPEN "AT+LOCK"
#define CMD_LOCK_CLOSE "AT-LOCK"
#define CMD_BUZZER "AT+BUZZ"
#define CMD_STATUS "AT+STATUS"
#define CMD_SENSOR_READ "AT+SENSOR"

// Response Prefixes
#define RESPONSE_EVENT "EVENT:"
#define RESPONSE_STATUS "STATUS:"
#define RESPONSE_ERROR "ERROR:"
#define RESPONSE_OK "OK"

// Event Types
#define EVENT_DOOR_CLOSED "DOOR_CLOSED"
#define EVENT_UNAUTHORIZED "UNAUTHORIZED"
#define EVENT_SYSTEM_ERROR "SYSTEM_ERROR"

#endif // PINS_CONFIG_H
