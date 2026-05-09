#ifndef PARCEL_BOX_COMM_H
#define PARCEL_BOX_COMM_H

#include "FirebaseManager.h"
#include "EspNowManager.h"

// ============================================================================
// PARCEL BOX COMMUNICATION ORCHESTRATOR
// ============================================================================
// This class bridges ESP-NOW (device-to-device) and Firebase (cloud sync)
// allowing seamless real-time communication between:
// - Main ESP32 ↔ ESP32-CAM via ESP-NOW
// - Both devices ↔ Firebase for cloud synchronization and app control
// ============================================================================

class ParcelBoxComm {
public:
  ParcelBoxComm();
  
  // Initialization
  void begin(const char* deviceId, const uint8_t* cameraMac);
  void setup();
  
  // Process communications (call in main loop)
  void handle();
  
  // Get managers (for advanced usage)
  FirebaseManager* getFirebaseManager() { return &firebaseManager; }
  EspNowManager* getEspNowManager() { return &espNowManager; }
  
  // QR Code handling (from camera)
  void onQrCodeScanned(const char* qrCode);
  
  // Lock control from app/Firebase
  void commandLock(uint8_t lockNum, bool unlock, uint16_t duration = 0);
  
  // Get camera status
  bool isCameraConnected();
  
  // Manual ESP-NOW peer management
  void addCameraPeer(const uint8_t* mac);
  void removeCameraPeer();
  
  // Statistics
  void printCommunicationStats();
  
private:
  FirebaseManager firebaseManager;
  EspNowManager espNowManager;
  
  uint8_t cameraMac[6];
  bool cameraConnected;
  unsigned long lastCameraHeartbeat;
  unsigned long lastStatusUpdate;
  
  // Configuration
  static const unsigned long CAMERA_HEARTBEAT_TIMEOUT = 30000;  // 30 seconds
  static const unsigned long STATUS_UPDATE_INTERVAL = 5000;     // 5 seconds
  
  // Helper callbacks
  void handleEspNowQrCode(const char* qrCode);
  void handleEspNowStatus(uint8_t battery, uint8_t signal, uint32_t uptime);
  void handleFirebaseCommand(const char* command);
};

#endif // PARCEL_BOX_COMM_H
