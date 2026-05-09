// ============================================================================
// EspNowCamera.cpp — DEPRECATED
// All ESP-NOW logic is inline in ParcelBoxEspCam.ino now.
// Kept to avoid compilation errors.
// Remove from project when fully migrated.
// ============================================================================
#include "EspNowCamera.h"

static EspNowCamera* globalEspNowCamera = nullptr;

EspNowCamera::EspNowCamera() : newMessageAvailable(false), qrCodesSent(0), commandsReceived(0) {
    memset(deviceId, 0, sizeof(deviceId));
    memset(mainEspMac, 0, 6);
    globalEspNowCamera = this;
}

void EspNowCamera::begin(const char*, const uint8_t*) {}
void EspNowCamera::sendMessage(EspNowMessage&) {}
void EspNowCamera::sendQrCode(const char*) {}
bool EspNowCamera::hasCommand() { return false; }
void EspNowCamera::processCommand() {}
void EspNowCamera::onLockCommand(uint8_t, uint8_t, uint16_t) {}
void EspNowCamera::printStatus() {}