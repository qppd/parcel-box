#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// ============================================================================
// ESP32 38-PIN CONFIGURATION - Smart Parcel Locker
// ============================================================================
// Single-MCU design: ESP32 controls ALL hardware directly.
// No Arduino Uno in the system.

// ============================================================================
// RELAY CONTROL (Solenoid Locks)
// ============================================================================
// WARNING: On standard ESP32-WROOM-32, GPIO 34 and 35 are INPUT-ONLY pins.
// Verify your ESP32 variant supports output on these GPIOs, or use an
// external I/O expander / different pins if relays do not respond.
#define RELAY_1_PIN 19           // GPIO34 - Relay CH1 (Parcel door lock)
#define RELAY_2_PIN 18           // GPIO35 - Relay CH2 (Payment box lock)

// ============================================================================
// SIM800L GSM MODULE (UART2)
// ============================================================================
#define SIM800L_RX_PIN 16        // GPIO16 - ESP32 RX from SIM800L TX
#define SIM800L_TX_PIN 17        // GPIO17 - ESP32 TX to SIM800L RX
#define SIM800L_RST_PIN 5        // GPIO5  - SIM800L hardware reset
#define BAUD_SIM800L 9600        // SIM800L default baud rate

// ============================================================================
// REED SWITCHES (Door Sensors)
// ============================================================================
#define DOOR_SENSOR_1_PIN 12     // GPIO12 - Parcel door reed switch
#define DOOR_SENSOR_2_PIN 14     // GPIO14 - Payment box reed switch

// ============================================================================
// BUZZER (LEDC PWM)
// ============================================================================
#define BUZZER_PIN 23            // GPIO23 - Piezo buzzer
#define BUZZER_RESOLUTION 8      // LEDC resolution bits (used with ledcAttach)

// ============================================================================
// I2C LCD DISPLAY (20x4)
// ============================================================================
#define LCD_SDA_PIN 21           // GPIO21 - I2C Data (default ESP32 SDA)
#define LCD_SCL_PIN 22           // GPIO22 - I2C Clock (default ESP32 SCL)
#define LCD_I2C_ADDRESS 0x27     // I2C address for 20x4 LCD
#define LCD_COLS 20              // LCD columns
#define LCD_ROWS 4               // LCD rows

// ============================================================================
// REED SWITCHES (Door Sensors)
// ============================================================================
// ============================================================================
#define BAUD_SERIAL 115200       // USB Serial debug + test commands

// ============================================================================
// TIMING CONSTANTS (milliseconds)
// ============================================================================
#define LOCK_OPERATION_DELAY 500      // Delay between lock operations
#define SMS_COOLDOWN 3000             // Prevent SMS spam (3s)
#define QR_SCAN_DEBOUNCE 500

#endif // PINS_CONFIG_H
