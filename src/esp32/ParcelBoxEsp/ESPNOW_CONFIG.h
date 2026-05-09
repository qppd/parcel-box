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

// ============================================================================
// CAMERA MAC ADDRESS
// ============================================================================
#define CAM_MAC_0 0xA0
#define CAM_MAC_1 0xDD
#define CAM_MAC_2 0x6C
#define CAM_MAC_3 0xAF
#define CAM_MAC_4 0x02
#define CAM_MAC_5 0x5C

#endif // ESPNOW_CONFIG_H