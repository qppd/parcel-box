// ============================================================================
// ParcelBoxComm.cpp — DEPRECATED
// All communication logic is now inline in ParcelBoxEsp.ino
// Kept to avoid compilation errors for the `comms` global object.
// Remove from project when fully migrated.
// ============================================================================
#include "ParcelBoxComm.h"

ParcelBoxComm::ParcelBoxComm() : cameraConnected(false), lastCameraHeartbeat(0), lastStatusUpdate(0) {
    memset(cameraMac, 0, 6);
}

void ParcelBoxComm::begin(const char*, const uint8_t*) {}
void ParcelBoxComm::setup() {}
void ParcelBoxComm::handle() {}
void ParcelBoxComm::onQrCodeScanned(const char*) {}
void ParcelBoxComm::commandLock(uint8_t, bool, uint16_t) {}
bool ParcelBoxComm::isCameraConnected() { return false; }
void ParcelBoxComm::addCameraPeer(const uint8_t*) {}
void ParcelBoxComm::removeCameraPeer() {}
void ParcelBoxComm::printCommunicationStats() {}
void ParcelBoxComm::handleEspNowQrCode(const char*) {}
void ParcelBoxComm::handleEspNowStatus(uint8_t, uint8_t, uint32_t) {}
void ParcelBoxComm::handleFirebaseCommand(const char*) {}