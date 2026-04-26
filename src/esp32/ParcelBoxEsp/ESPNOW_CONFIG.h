#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

// ============================================================================
// ESP-NOW SHARED CONFIGURATION - Smart Parcel Locker
// ============================================================================
// Include this file in BOTH sketches (EspCam and Main Esp)
// to ensure data structure consistency.

#include <Arduino.h>

// ============================================================================
// ESP-NOW SETTINGS
// ============================================================================
#define ESPNOW_MAX_PAYLOAD 32      // Maximum QR code length
#define ESPNOW_CHANNEL_WIFI 0      // 0 = use current WiFi channel (auto-sync)

// ============================================================================
// DATA PACKET STRUCTURE (MUST BE IDENTICAL IN BOTH SKETCHES)
// ============================================================================
// Use packed attribute to ensure consistent memory layout
struct __attribute__((packed)) QRDataPacket {
    char qrData[32];              // QR code payload (parcelId)
    uint32_t timestamp;           // Scan timestamp (millis())
    uint8_t senderMac[6];         // Sender's MAC address
};

// For backward compatibility with existing code
typedef QRDataPacket ESPNOW_QRPacket_t;

// ============================================================================
// REMINDER: Update Receiver MAC Address
// ============================================================================
// In ParcelBoxEspCam.ino, update the receiverMac[] array with your
// Main ESP32's MAC address. Get it by running:
//   Serial.println(WiFi.macAddress());
// on the main ESP32 after it connects to WiFi.
//
// Example:
//   uint8_t receiverMac[] = {0x7C, 0x9E, 0xBD, 0x15, 0x34, 0x56};
//
// ============================================================================

#endif // ESPNOW_CONFIG_H
