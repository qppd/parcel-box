#ifndef ESPNOW_CAMERA_H
#define ESPNOW_CAMERA_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ============================================================================
// ESP32-CAM ESP-NOW COMMUNICATION
// ============================================================================
// Minimal ESP-NOW implementation for ESP32-CAM
// Sends: QR codes via ESP-NOW to main ESP32
// Receives: Lock control commands from main ESP32
// ============================================================================

// Message types (must match EspNowManager.h)
#define MSG_TYPE_QR_SCAN        0
#define MSG_TYPE_LOCK_CMD       1
#define MSG_TYPE_STATUS         2
#define MSG_TYPE_CONFIG         3
#define MSG_TYPE_ACK            4

typedef struct {
  uint8_t type;
  char deviceId[32];
  uint32_t timestamp;
  uint8_t retries;
  union {
    char qrCode[64];
    struct {
      uint8_t lock;
      uint8_t command;
      uint16_t duration;
    } lockCmd;
  } payload;
} EspNowMessage;

class EspNowCamera {
public:
  EspNowCamera();
  
  // Initialization
  void begin(const char* deviceId, const uint8_t* mainEspMac);
  
  // Send QR scan to main ESP32
  void sendQrCode(const char* qrCode);
  
  // Check for incoming lock commands
  bool hasCommand();
  void processCommand();
  
  // Lock command callback (user implements)
  void onLockCommand(uint8_t lockNum, uint8_t command, uint16_t duration);
  
  // Statistics
  void printStatus();

private:
  char deviceId[32];
  uint8_t mainEspMac[6];
  bool newMessageAvailable;

  uint32_t qrCodesSent;

  void sendMessage(EspNowMessage& msg);

public:
  EspNowMessage lastMessage;
  uint32_t commandsReceived;
};

#endif // ESPNOW_CAMERA_H
