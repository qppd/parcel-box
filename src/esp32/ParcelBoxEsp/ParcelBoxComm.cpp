#include "ParcelBoxComm.h"

ParcelBoxComm::ParcelBoxComm() : cameraConnected(false), lastCameraHeartbeat(0), lastStatusUpdate(0) {
  memset(cameraMac, 0, 6);
}

void ParcelBoxComm::begin(const char* deviceId, const uint8_t* cameraMacAddr) {
  Serial.println("[COMM] Initializing Parcel Box Communication System...");
  
  // Store camera MAC
  memcpy(cameraMac, cameraMacAddr, 6);
  
  // Initialize Firebase Manager
  Serial.println("[COMM] → Initializing Firebase...");
  firebaseManager.setDeviceId(deviceId);
  firebaseManager.begin();
  
  // Initialize ESP-NOW Manager
  Serial.println("[COMM] → Initializing ESP-NOW...");
  espNowManager.begin(deviceId);
  
  // Add camera as peer
  addCameraPeer(cameraMac);
  
  Serial.println("[COMM] ✅ Communication system ready!");
  printCommunicationStats();
}

void ParcelBoxComm::setup() {
  // Any additional setup if needed
  lastStatusUpdate = millis();
}

void ParcelBoxComm::handle() {
  // Process Firebase streams for incoming commands from app
  if (firebaseManager.isReady()) {
    firebaseManager.handleStreams();
  }
  
  // Check camera heartbeat
  if (isCameraConnected() && millis() - lastCameraHeartbeat > CAMERA_HEARTBEAT_TIMEOUT) {
    Serial.println("[COMM] ⚠️ Camera heartbeat timeout - device may be offline");
    cameraConnected = false;
  }
  
  // Periodic status update to Firebase
  if (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
    lastStatusUpdate = millis();
    
    // Update device status with camera connection state
    firebaseManager.updateDeviceStatus(
      "main_esp32",  // TODO: Use actual device ID
      WiFi.status() == WL_CONNECTED,
      firebaseManager.isReady(),
      0.0,  // temperature
      0     // humidity
    );
    
    // Send periodic status to camera via ESP-NOW
    if (cameraConnected) {
      uint32_t uptime = millis() / 1000;
      espNowManager.sendStatus(100, WiFi.RSSI() + 100, uptime);  // Simplified battery/signal
    }
  }
}

void ParcelBoxComm::onQrCodeScanned(const char* qrCode) {
  Serial.println("[COMM] 📸 QR Code received: " + String(qrCode));
  
  // Log to Firebase
  if (firebaseManager.isReady()) {
    firebaseManager.logParcelEvent("main_esp32", qrCode, "QR_SCANNED_FROM_CAMERA");
  }
  
  // Process locally (validate and open locks)
  handleEspNowQrCode(qrCode);
}

void ParcelBoxComm::commandLock(uint8_t lockNum, bool unlock, uint16_t duration) {
  Serial.printf("[COMM] 🔓 Commanding lock%d: %s (duration=%d ms)\n",
                lockNum, unlock ? "UNLOCK" : "LOCK", duration);
  
  // Send to camera via ESP-NOW
  if (cameraConnected) {
    espNowManager.sendLockCommand(lockNum, unlock ? 1 : 0, duration);
  }
  
  // Log to Firebase
  if (firebaseManager.isReady()) {
    String event = unlock ? "LOCK_OPENED" : "LOCK_CLOSED";
    firebaseManager.logParcelEvent("main_esp32", "lock_" + String(lockNum), event);
  }
}

bool ParcelBoxComm::isCameraConnected() {
  return cameraConnected && espNowManager.isPeerConnected(cameraMac);
}

void ParcelBoxComm::addCameraPeer(const uint8_t* mac) {
  espNowManager.addPeer(mac);
  Serial.printf("[COMM] ✅ Camera peer added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void ParcelBoxComm::removeCameraPeer() {
  espNowManager.removePeer(cameraMac);
  cameraConnected = false;
  Serial.println("[COMM] Camera peer removed");
}

void ParcelBoxComm::handleEspNowQrCode(const char* qrCode) {
  // This will be called when camera sends QR via ESP-NOW
  // Integration point with main parcel processing logic
  Serial.println("[COMM] → Queuing QR for parcel validation: " + String(qrCode));
  
  // TODO: Call main loop's parcel validation function
  // validateAndOpenLocks(qrCode);
}

void ParcelBoxComm::handleEspNowStatus(uint8_t battery, uint8_t signal, uint32_t uptime) {
  // Camera status received
  if (!cameraConnected) {
    Serial.println("[COMM] ✅ Camera connected!");
    cameraConnected = true;
  }
  lastCameraHeartbeat = millis();
  
  Serial.printf("[COMM] 📸 Camera Status: Battery=%d%%, Signal=%d%%, Uptime=%lu sec\n",
                battery, signal, uptime);
}

void ParcelBoxComm::handleFirebaseCommand(const char* command) {
  // Process commands coming from Firebase
  Serial.println("[COMM] 📱 Firebase command received: " + String(command));
  
  // TODO: Parse and execute Firebase commands
  // Examples: "unlock_all", "lock_emergency", "sync_status", etc.
}

void ParcelBoxComm::printCommunicationStats() {
  Serial.println("\n[COMM] === Communication Statistics ===");
  Serial.printf("[COMM] Camera Connected: %s\n", cameraConnected ? "YES" : "NO");
  Serial.printf("[COMM] Firebase Ready: %s\n", firebaseManager.isReady() ? "YES" : "NO");
  espNowManager.printPeerInfo();
  Serial.println("[COMM] ===================================\n");
}
