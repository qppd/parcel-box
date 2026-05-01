#ifndef ESPNOW_CONFIG_H
#define ESPNOW_CONFIG_H

// ... existing content ...

// ============================================================================
// ESP-NOW SETTINGS
// ============================================================================
#define ESPN0W_DUPLICATE_COOLDOWN 2000  // 2 seconds cooldown for duplicate scans
#define ESPNOW_MAX_PAYLOAD 32      // Maximum QR code length
#define ESPNOW_CHANNEL_WIFI 0      // 0 = use current WiFi channel (auto-sync)

// ... rest of file content ...