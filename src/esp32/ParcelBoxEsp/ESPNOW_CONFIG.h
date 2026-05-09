#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ============================================================================
// ESP-NOW QR PACKET STRUCTURE
// ============================================================================
// MUST match the ESP32-CAM's packet exactly (packed for ABI compat)
typedef struct __attribute__((packed)) {
  char qrData[32];        // QR payload (parcelId)
  uint32_t timestamp;     // Scan timestamp (millis)
  uint8_t camMac[6];      // ESP32-CAM MAC address
} ESPNOW_QRPacket_t;

// ============================================================================
// ESP-NOW SETTINGS
// ============================================================================
#define ESPNOW_DUPLICATE_COOLDOWN 2000     // 2 sec cooldown for duplicate scans
#define ESPNOW_MAX_PAYLOAD 32              // Max QR code length
#define ESPNOW_RETRY_COUNT      2          // Number of send retries
#define ESPNOW_ACK_TIMEOUT_MS   500        // Wait for ACK before retry

// ============================================================================
// CAMERA MAC ADDRESS (ESP32-CAM)
// ============================================================================
#define CAM_MAC_0 0xA0
#define CAM_MAC_1 0xDD
#define CAM_MAC_2 0x6C
#define CAM_MAC_3 0xAF
#define CAM_MAC_4 0x02
#define CAM_MAC_5 0x5C

#endif // ESPNOW_CONFIG_H