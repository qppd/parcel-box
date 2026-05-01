#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

#include <Arduino.h>
#include <esp_now.h>

// ============================================================================
// ESP-NOW STRUCTURES
// ============================================================================
// Data structure for ESP-NOW QR code packets
typedef struct {
  char qrData[32];  // 32-byte payload for QR code data (ESP-NOW max payload)
  uint8_t packetId;   // Packet sequence number for duplicate detection
} ESPNOW_QRPacket_t;

// ============================================================================
// ESP-NOW SETTINGS
// ============================================================================
#define ESPNOW_DUPLICATE_COOLDOWN 2000  // 2 seconds cooldown for duplicate scans
#define ESPNOW_MAX_PAYLOAD 32      // Maximum QR code length
#define ESPNOW_CHANNEL_WIFI 0      // 0 = use current WiFi channel (auto-sync)

#endif // ESPNOW_CONFIG_H