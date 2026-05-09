#include "EspNowCamera.h"

static EspNowCamera* globalEspNowCamera = nullptr;

static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("[CAM-ESP-NOW] Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "✅ SUCCESS" : "❌ FAILED");
}

static void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (globalEspNowCamera == nullptr || len != sizeof(EspNowMessage)) return;
  
  EspNowMessage receivedMsg;
  memcpy(&receivedMsg, incomingData, sizeof(EspNowMessage));
  
  // Only process lock commands for camera
  if (receivedMsg.type == MSG_TYPE_LOCK_CMD) {
    Serial.printf("[CAM-ESP-NOW] 🔓 Lock command received: Lock%d, Cmd=%d\n",
                  receivedMsg.payload.lockCmd.lock,
                  receivedMsg.payload.lockCmd.command);
    globalEspNowCamera->onLockCommand(
      receivedMsg.payload.lockCmd.lock,
      receivedMsg.payload.lockCmd.command,
      receivedMsg.payload.lockCmd.duration
    );
    globalEspNowCamera->commandsReceived++;
  }
  
  globalEspNowCamera->lastMessage = receivedMsg;
}

EspNowCamera::EspNowCamera() : newMessageAvailable(false), qrCodesSent(0), commandsReceived(0) {
  memset(deviceId, 0, sizeof(deviceId));
  memset(mainEspMac, 0, 6);
  globalEspNowCamera = this;
}

void EspNowCamera::begin(const char* id, const uint8_t* mainMac) {
  strncpy(deviceId, id, sizeof(deviceId) - 1);
  memcpy(mainEspMac, mainMac, 6);
  
  Serial.println("[CAM-ESP-NOW] Initializing camera ESP-NOW...");
  
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("[CAM-ESP-NOW] ❌ Failed to initialize");
    return;
  }
  
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  
  // Add main ESP32 as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mainEspMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  
  Serial.println("[CAM-ESP-NOW] ✅ Ready");
  Serial.printf("[CAM-ESP-NOW] Main ESP32: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mainMac[0], mainMac[1], mainMac[2], mainMac[3], mainMac[4], mainMac[5]);
}

void EspNowCamera::sendMessage(EspNowMessage& msg) {
  strncpy(msg.deviceId, deviceId, sizeof(msg.deviceId) - 1);
  msg.timestamp = millis();
  
  esp_err_t result = esp_now_send(mainEspMac, (uint8_t *) &msg, sizeof(msg));
  if (result == ESP_OK) {
    Serial.println("[CAM-ESP-NOW] ✅ Message sent");
  } else {
    Serial.printf("[CAM-ESP-NOW] ❌ Send failed: %d\n", result);
  }
}

void EspNowCamera::sendQrCode(const char* qrCode) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_QR_SCAN;
  strncpy(msg.payload.qrCode, qrCode, sizeof(msg.payload.qrCode) - 1);
  
  Serial.println("[CAM-ESP-NOW] 📸 Sending QR: " + String(qrCode));
  sendMessage(msg);
  qrCodesSent++;
}

bool EspNowCamera::hasCommand() {
  return newMessageAvailable;
}

void EspNowCamera::processCommand() {
  newMessageAvailable = false;
  // Command already processed in callback
}

void EspNowCamera::onLockCommand(uint8_t lockNum, uint8_t command, uint16_t duration) {
  // Default implementation - override in derived class or user code
  Serial.println("[CAM-ESP-NOW] Lock command callback - override in your code");
}

void EspNowCamera::printStatus() {
  Serial.println("\n[CAM-ESP-NOW] === Camera ESP-NOW Status ===");
  Serial.printf("[CAM-ESP-NOW] Device ID: %s\n", deviceId);
  Serial.printf("[CAM-ESP-NOW] QR Codes Sent: %lu\n", qrCodesSent);
  Serial.printf("[CAM-ESP-NOW] Commands Received: %lu\n", commandsReceived);
  Serial.println("[CAM-ESP-NOW] ====================================\n");
}
