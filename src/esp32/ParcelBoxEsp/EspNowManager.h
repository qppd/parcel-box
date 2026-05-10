#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// ============================================================================
// ESP-NOW MESSAGE STRUCTURE
// ============================================================================
// All ESP-NOW messages follow this struct for type safety
typedef struct {
  uint8_t type;           // Message type (0=QR_SCAN, 1=LOCK_CMD, 2=STATUS, 3=CONFIG)
  char deviceId[32];      // Source device ID
  uint32_t timestamp;     // Milliseconds timestamp
  uint8_t retries;        // Number of retries for reliability
  
  // Payload (max 200 bytes for data)
  union {
    char qrCode[64];          // For QR scans
    struct {
      uint8_t lock;           // Lock number (1-2)
      uint8_t command;        // 0=close, 1=open
      uint16_t duration;      // Duration in milliseconds
    } lockCmd;
    struct {
      uint8_t battery;        // Battery percentage
      uint8_t signalStrength; // WiFi/ESP-NOW signal
      uint32_t uptime;        // System uptime
    } status;
    char configData[128];     // Config JSON or text
  } payload;
} EspNowMessage;

// Message types
#define MSG_TYPE_QR_SCAN        0
#define MSG_TYPE_LOCK_CMD       1
#define MSG_TYPE_STATUS         2
#define MSG_TYPE_CONFIG         3
#define MSG_TYPE_ACK            4

// ============================================================================
// ESP-NOW MANAGER CLASS
// ============================================================================
class EspNowManager {
public:
  EspNowManager();
  
  // Initialization
  void begin(const char* deviceId);
  void addPeer(const uint8_t* peerMac);
  void removePeer(const uint8_t* peerMac);
  
  // Sending data
  void sendQrCode(const char* qrCode);
  void sendLockCommand(uint8_t lockNum, uint8_t command, uint16_t duration = 0);
  void sendStatus(uint8_t battery, uint8_t signal, uint32_t uptime);
  void sendConfigUpdate(const char* configJson);
  void sendAck(uint32_t messageId);
  
  // Receiving data (non-blocking)
  bool hasNewMessage();
  EspNowMessage getLastMessage();
  void clearMessageBuffer();
  
  // Callbacks (user implements these)
  void onQrScanReceived(const char* qrCode);
  void onLockCommandReceived(uint8_t lock, uint8_t command, uint16_t duration);
  void onStatusReceived(uint8_t battery, uint8_t signal, uint32_t uptime);
  void onConfigReceived(const char* configJson);
  
  // Health check
  bool isPeerConnected(const uint8_t* peerMac);
  void printPeerInfo();
  
  // Reliability
  uint32_t getMessagesSent() { return messagesSent; }
  uint32_t getMessagesReceived() { return messagesReceived; }
  uint32_t getMessagesFailed() { return messagesFailed; }

private:
  char deviceId[32];
  bool newMessageAvailable;

  // Helper
  void sendMessage(EspNowMessage& msg);

public:
  EspNowMessage lastMessage;

  // Statistics
  uint32_t messagesSent;
  uint32_t messagesReceived;
  uint32_t messagesFailed;
  
  };

#endif // ESPNOW_MANAGER_H
