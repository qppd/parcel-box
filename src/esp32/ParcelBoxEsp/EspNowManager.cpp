#include "EspNowManager.h"

// Global instance for callback access
static EspNowManager* globalEspNowManager = nullptr;

// ============================================================================
// ESP-NOW CALLBACKS (Static for C-style callback compatibility)
// ============================================================================
static void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  if (globalEspNowManager == nullptr) return;
  
  Serial.print("[ESP-NOW] Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
  
  if (status != ESP_NOW_SEND_SUCCESS) {
    globalEspNowManager->messagesFailed++;
  }
}

static void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (globalEspNowManager == nullptr) return;
  
  // Only accept messages of expected size
  if (len != sizeof(EspNowMessage)) {
    Serial.printf("[ESP-NOW] ⚠️ Invalid message size: %d (expected %d)\n", len, sizeof(EspNowMessage));
    return;
  }
  
  // Copy received data to struct
  EspNowMessage receivedMsg;
  memcpy(&receivedMsg, incomingData, sizeof(EspNowMessage));
  
  Serial.printf("[ESP-NOW] Message received from %02X:%02X:%02X:%02X:%02X:%02X\n",
                recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
                recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
  
  // Process message based on type
  switch (receivedMsg.type) {
    case MSG_TYPE_QR_SCAN:
      Serial.println("[ESP-NOW] 📸 QR Code received: " + String(receivedMsg.payload.qrCode));
      globalEspNowManager->onQrScanReceived(receivedMsg.payload.qrCode);
      break;
      
    case MSG_TYPE_LOCK_CMD:
      Serial.printf("[ESP-NOW] 🔓 Lock command: Lock%d, Cmd=%d, Duration=%d ms\n",
                    receivedMsg.payload.lockCmd.lock,
                    receivedMsg.payload.lockCmd.command,
                    receivedMsg.payload.lockCmd.duration);
      globalEspNowManager->onLockCommandReceived(
        receivedMsg.payload.lockCmd.lock,
        receivedMsg.payload.lockCmd.command,
        receivedMsg.payload.lockCmd.duration
      );
      break;
      
    case MSG_TYPE_STATUS:
      Serial.printf("[ESP-NOW] 📊 Status: Battery=%d%%, Signal=%d%%, Uptime=%lu sec\n",
                    receivedMsg.payload.status.battery,
                    receivedMsg.payload.status.signalStrength,
                    receivedMsg.payload.status.uptime);
      globalEspNowManager->onStatusReceived(
        receivedMsg.payload.status.battery,
        receivedMsg.payload.status.signalStrength,
        receivedMsg.payload.status.uptime
      );
      break;
      
    case MSG_TYPE_CONFIG:
      Serial.println("[ESP-NOW] ⚙️ Config update: " + String(receivedMsg.payload.configData));
      globalEspNowManager->onConfigReceived(receivedMsg.payload.configData);
      break;
      
    case MSG_TYPE_ACK:
      Serial.println("[ESP-NOW] ✅ Acknowledgment received");
      break;
      
    default:
      Serial.printf("[ESP-NOW] ❓ Unknown message type: %d\n", receivedMsg.type);
  }
  
  globalEspNowManager->messagesReceived++;
  globalEspNowManager->lastMessage = receivedMsg;
}

// ============================================================================
// ESPNOWMANAGER IMPLEMENTATION
// ============================================================================
EspNowManager::EspNowManager() {
  memset(deviceId, 0, sizeof(deviceId));
  newMessageAvailable = false;
  messagesSent = 0;
  messagesReceived = 0;
  messagesFailed = 0;
  globalEspNowManager = this;
}

void EspNowManager::begin(const char* id) {
  strncpy(deviceId, id, sizeof(deviceId) - 1);
  
  Serial.println("[ESP-NOW] Initializing ESP-NOW...");
  
  // WiFi must be in station or AP mode
  WiFi.mode(WIFI_STA);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ❌ Failed to initialize ESP-NOW");
    return;
  }
  
  // Register callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  
  Serial.println("[ESP-NOW] ✅ Initialized successfully");
  Serial.printf("[ESP-NOW] Device ID: %s\n", deviceId);
  Serial.printf("[ESP-NOW] MAC Address: %s\n", WiFi.macAddress().c_str());
}

void EspNowManager::addPeer(const uint8_t* peerMac) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ESP-NOW] ❌ Failed to add peer");
    return;
  }
  
  Serial.printf("[ESP-NOW] ✅ Peer added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                peerMac[0], peerMac[1], peerMac[2], peerMac[3], peerMac[4], peerMac[5]);
}

void EspNowManager::removePeer(const uint8_t* peerMac) {
  esp_now_del_peer(peerMac);
  Serial.printf("[ESP-NOW] Peer removed: %02X:%02X:%02X:%02X:%02X:%02X\n",
                peerMac[0], peerMac[1], peerMac[2], peerMac[3], peerMac[4], peerMac[5]);
}

void EspNowManager::sendMessage(EspNowMessage& msg) {
  // Fill common fields
  strncpy(msg.deviceId, deviceId, sizeof(msg.deviceId) - 1);
  msg.timestamp = millis();
  
  // Get broadcast MAC (FF:FF:FF:FF:FF:FF) for testing, or specific peer in production
  uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  
  esp_err_t result = esp_now_send(broadcastMac, (uint8_t *) &msg, sizeof(msg));
  
  if (result == ESP_OK) {
    messagesSent++;
    Serial.printf("[ESP-NOW] ✅ Message sent (Type=%d, Size=%d bytes)\n", msg.type, sizeof(msg));
  } else {
    messagesFailed++;
    Serial.printf("[ESP-NOW] ❌ Failed to send message: %d\n", result);
  }
}

void EspNowManager::sendQrCode(const char* qrCode) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_QR_SCAN;
  strncpy(msg.payload.qrCode, qrCode, sizeof(msg.payload.qrCode) - 1);
  
  Serial.println("[ESP-NOW] 📸 Sending QR code: " + String(qrCode));
  sendMessage(msg);
}

void EspNowManager::sendLockCommand(uint8_t lockNum, uint8_t command, uint16_t duration) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_LOCK_CMD;
  msg.payload.lockCmd.lock = lockNum;
  msg.payload.lockCmd.command = command;
  msg.payload.lockCmd.duration = duration;
  
  Serial.printf("[ESP-NOW] 🔓 Sending lock command: Lock%d, Cmd=%d, Duration=%d ms\n",
                lockNum, command, duration);
  sendMessage(msg);
}

void EspNowManager::sendStatus(uint8_t battery, uint8_t signal, uint32_t uptime) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_STATUS;
  msg.payload.status.battery = battery;
  msg.payload.status.signalStrength = signal;
  msg.payload.status.uptime = uptime;
  
  Serial.printf("[ESP-NOW] 📊 Sending status: Battery=%d%%, Signal=%d%%, Uptime=%lu sec\n",
                battery, signal, uptime);
  sendMessage(msg);
}

void EspNowManager::sendConfigUpdate(const char* configJson) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_CONFIG;
  strncpy(msg.payload.configData, configJson, sizeof(msg.payload.configData) - 1);
  
  Serial.println("[ESP-NOW] ⚙️ Sending config: " + String(configJson));
  sendMessage(msg);
}

void EspNowManager::sendAck(uint32_t messageId) {
  EspNowMessage msg = {};
  msg.type = MSG_TYPE_ACK;
  
  Serial.printf("[ESP-NOW] ✅ Sending acknowledgment for message %lu\n", messageId);
  sendMessage(msg);
}

bool EspNowManager::hasNewMessage() {
  return newMessageAvailable;
}

EspNowMessage EspNowManager::getLastMessage() {
  newMessageAvailable = false;
  return lastMessage;
}

void EspNowManager::clearMessageBuffer() {
  newMessageAvailable = false;
  memset(&lastMessage, 0, sizeof(EspNowMessage));
}

bool EspNowManager::isPeerConnected(const uint8_t* peerMac) {
  esp_now_peer_info_t peerInfo;
  esp_err_t result = esp_now_get_peer(peerMac, &peerInfo);
  return result == ESP_OK;
}

void EspNowManager::printPeerInfo() {
  Serial.println("[ESP-NOW] Peer Information:");
  Serial.printf("  Messages Sent: %lu\n", messagesSent);
  Serial.printf("  Messages Received: %lu\n", messagesReceived);
  Serial.printf("  Messages Failed: %lu\n", messagesFailed);
  
  if (messagesReceived + messagesSent > 0) {
    float successRate = ((float)(messagesSent - messagesFailed) / (messagesSent)) * 100.0;
    Serial.printf("  Success Rate: %.1f%%\n", successRate);
  }
}

// Default callback implementations (weak - can be overridden)
void EspNowManager::onQrScanReceived(const char* qrCode) {
  // Default implementation - can be overridden in derived class
  Serial.println("[ESP-NOW] QR Scan callback - override in your code");
}

void EspNowManager::onLockCommandReceived(uint8_t lock, uint8_t command, uint16_t duration) {
  // Default implementation - can be overridden in derived class
  Serial.println("[ESP-NOW] Lock command callback - override in your code");
}

void EspNowManager::onStatusReceived(uint8_t battery, uint8_t signal, uint32_t uptime) {
  // Default implementation - can be overridden in derived class
  Serial.println("[ESP-NOW] Status callback - override in your code");
}

void EspNowManager::onConfigReceived(const char* configJson) {
  // Default implementation - can be overridden in derived class
  Serial.println("[ESP-NOW] Config callback - override in your code");
}
