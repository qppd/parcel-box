// ============================================================================
// FirebaseManager.cpp — DEPRECATED
// All Firebase logic is now inline in ParcelBoxEsp.ino
// Kept to avoid compilation errors for any remaining includes.
// Remove from project when fully migrated.
// ============================================================================
#include "FirebaseManager.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseManager* globalFirebaseManager = nullptr;

FirebaseManager::FirebaseManager() {
    globalFirebaseManager = this;
}

void FirebaseManager::begin() {
    // No-op: see ParcelBoxEsp.ino initializeFirebase()
}

void FirebaseManager::setDeviceId(const String& id) {}

void FirebaseManager::initializeStreams() {}
void FirebaseManager::handleStreams() {}

void FirebaseManager::setLockCommandCallback(LockCommandCallback cb) {}
void FirebaseManager::setEmergencyCallback(EmergencyCallback cb) {}

void FirebaseManager::updateDeviceStatus(const String&, bool, bool, float, int) {}
void FirebaseManager::updateLockStatus(const String&, bool, bool, bool, bool) {}
void FirebaseManager::logParcelEvent(const String&, const String&, const String&) {}

void FirebaseManager::commandStreamCallback(MultiPathStream) {}
void FirebaseManager::commandStreamTimeoutCallback(bool) {}

bool FirebaseManager::isReady() {
    extern FirebaseData fbdo;
    extern FirebaseConfig fbConfig;
    extern FirebaseAuth auth;
    // Only used for setup — returns true if the (now-global) Firebase is ready
    return Firebase.ready();
}