// ============================================================================
// EspNowManager.cpp — DEPRECATED
// All ESP-NOW logic is now inline in ParcelBoxEsp.ino
// Kept to avoid compilation errors.
// Remove from project when fully migrated.
// ============================================================================
#include "EspNowManager.h"

EspNowManager::EspNowManager() {
    memset(deviceId, 0, sizeof(deviceId));
    newMessageAvailable = false;
    messagesSent = 0;
    messagesReceived = 0;
    messagesFailed = 0;
}

void EspNowManager::begin(const char* id) {}
void EspNowManager::addPeer(const uint8_t*) {}
void EspNowManager::removePeer(const uint8_t*) {}
void EspNowManager::sendQrCode(const char*) {}
void EspNowManager::sendLockCommand(uint8_t, uint8_t, uint16_t) {}
void EspNowManager::sendStatus(uint8_t, uint8_t, uint32_t) {}
void EspNowManager::sendConfigUpdate(const char*) {}
void EspNowManager::sendAck(uint32_t) {}
bool EspNowManager::hasNewMessage() { return false; }
EspNowMessage EspNowManager::getLastMessage() { EspNowMessage m = {}; return m; }
void EspNowManager::clearMessageBuffer() {}
void EspNowManager::onQrScanReceived(const char*) {}
void EspNowManager::onLockCommandReceived(uint8_t, uint8_t, uint16_t) {}
void EspNowManager::onStatusReceived(uint8_t, uint8_t, uint32_t) {}
void EspNowManager::onConfigReceived(const char*) {}
bool EspNowManager::isPeerConnected(const uint8_t*) { return false; }
void EspNowManager::printPeerInfo() {}