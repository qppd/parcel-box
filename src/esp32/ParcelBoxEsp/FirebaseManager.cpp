#include "FirebaseManager.h"

// Global instance pointer for callback access
FirebaseManager* globalFirebaseManager = nullptr;

FirebaseManager::FirebaseManager() {
    deviceId = "";
    commandStreamActive = false;
    lastStatusUpdate = 0;
    lastStreamCheck = 0;
    globalFirebaseManager = this;
}

void FirebaseManager::setDeviceId(const String& id) {
    deviceId = id;
    Serial.println("[FB] Device ID set to: " + deviceId);
}

void FirebaseManager::begin() {
    Serial.println("[FB] Initializing Firebase Manager...");
    
    // Configure Firebase
    config.host = ParcelBoxFirebaseConfig::getFirebaseHost();
    config.database_url = ParcelBoxFirebaseConfig::getDatabaseURL();
    config.signer.tokens.legacy_token = ParcelBoxFirebaseConfig::getFirebaseAuth();
    config.timeout.serverResponse = 10 * 1000;  // 10 seconds
    config.timeout.rtdbStreamReconnect = 1 * 1000;  // 1 second
    config.timeout.rtdbStreamError = 3 * 1000;  // 3 seconds
    
    // Configure buffer sizes
    fbdo.setBSSLBufferSize(2048, 1024);
    fbdo.setResponseSize(2048);
    commandStream.setBSSLBufferSize(2048, 1024);
    commandStream.setResponseSize(2048);
    
    // Begin Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    Serial.println("[FB] Firebase.begin() called - waiting for ready...");
    
    // Wait for Firebase to be ready (max 10 seconds)
    unsigned long startTime = millis();
    while (!Firebase.ready() && millis() - startTime < 10000) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    
    if (Firebase.ready()) {
        Serial.println("[FB] ✅ Firebase CONNECTED!");

        // Initialize streams for real-time communication
        initializeStreams();
    } else {
        Serial.println("[FB] ❌ Firebase FAILED - check credentials and network");
    }
}

void FirebaseManager::setLockCommandCallback(LockCommandCallback cb) {
    lockCommandCb = cb;
    Serial.println("[FB] Lock command callback registered");
}

void FirebaseManager::setEmergencyCallback(EmergencyCallback cb) {
    emergencyCb = cb;
    Serial.println("[FB] Emergency callback registered");
}

void FirebaseManager::initializeStreams() {
    Serial.println("[FB] Initializing Firebase streams...");
    
    // Initialize command stream for real-time control from app
    initializeCommandStream();
    
    Serial.println("[FB] Streams initialization completed!");
}

void FirebaseManager::initializeCommandStream() {
    if (deviceId.length() == 0) {
        Serial.println("[FB] ❌ Cannot initialize command stream - device ID not set");
        return;
    }
    
    // Path: /devices/{device_id}/commands
    // App will write commands to this path, ESP32 listens via stream
    String commandPath = String(ParcelBoxFirebaseConfig::getDeviceStatusPath()) + "/" + deviceId + "/commands";
    Serial.println("[FB] Starting command stream for path: " + commandPath);
    
    if (!Firebase.RTDB.beginMultiPathStream(&commandStream, commandPath)) {
        Serial.printf("[FB] ❌ Command stream initialization failed: %s\n", commandStream.errorReason().c_str());
        commandStreamActive = false;
    } else {
        Firebase.RTDB.setMultiPathStreamCallback(&commandStream, commandStreamCallback, commandStreamTimeoutCallback);
        commandStreamActive = true;
        Serial.println("[FB] ✅ Command stream initialized successfully!");
    }
}

void FirebaseManager::handleStreams() {
    // Handle command stream for real-time updates from app
    if (commandStreamActive && !Firebase.RTDB.readStream(&commandStream)) {
        Serial.printf("[FB] ⚠️ Command stream error: %s\n", commandStream.errorReason().c_str());
        
        // Reinitialize if stream encounters error
        if (commandStream.httpCode() != FIREBASE_ERROR_HTTP_CODE_OK) {
            Serial.println("[FB] Attempting to reinitialize command stream...");
            initializeCommandStream();
        }
    }
    
    // Periodic stream health check
    if (millis() - lastStreamCheck >= STREAM_HEALTH_CHECK_INTERVAL) {
        lastStreamCheck = millis();
        Serial.printf("[FB] Stream health check - Command stream: %s\n", 
                      commandStreamActive ? "Active" : "Inactive");
    }
}

void FirebaseManager::updateDeviceStatus(const String& deviceId, bool wifiConnected, bool fbConnected, float temperature, int humidity) {
    if (!Firebase.ready()) {
        return;
    }
    
    // Only update every STATUS_UPDATE_INTERVAL
    if (millis() - lastStatusUpdate < STATUS_UPDATE_INTERVAL) {
        return;
    }
    lastStatusUpdate = millis();
    
    String statusPath = String(ParcelBoxFirebaseConfig::getDeviceStatusPath()) + "/" + deviceId;
    
    FirebaseJson json;
    json.set("device_id", deviceId);
    json.set("wifi_connected", wifiConnected);
    json.set("firebase_connected", fbConnected);
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("last_heartbeat", millis());
    json.set("timestamp/.sv", "timestamp");

    if (Firebase.RTDB.setJSON(&fbdo, statusPath, &json)) {
        Serial.println("[FB] ✅ Device status updated");
    } else {
        Serial.printf("[FB] ❌ Failed to update device status: %s\n", fbdo.errorReason().c_str());
    }
}

void FirebaseManager::updateLockStatus(const String& deviceId, bool lock1Open, bool lock2Open, bool door1Open, bool door2Open) {
    if (!Firebase.ready()) {
        return;
    }
    
    String locksPath = String(ParcelBoxFirebaseConfig::getLocksStatusPath()) + "/" + deviceId;
    
    FirebaseJson json;
    json.set("lock1", lock1Open ? "open" : "closed");
    json.set("lock2", lock2Open ? "open" : "closed");
    json.set("door1", door1Open ? "open" : "closed");
    json.set("door2", door2Open ? "open" : "closed");
    json.set("timestamp/.sv", "timestamp");
    
    if (Firebase.RTDB.setJSON(&fbdo, locksPath, &json)) {
        Serial.println("[FB] ✅ Lock status updated");
    } else {
        Serial.printf("[FB] ❌ Failed to update lock status: %s\n", fbdo.errorReason().c_str());
    }
}

void FirebaseManager::logParcelEvent(const String& deviceId, const String& parcelId, const String& event) {
    if (!Firebase.ready()) {
        return;
    }
    
    String logPath = String(ParcelBoxFirebaseConfig::getHistoryPath()) + "/" + deviceId + "/" + parcelId;
    
    FirebaseJson json;
    json.set("event", event);
    json.set("parcel_id", parcelId);
    json.set("timestamp/.sv", "timestamp");
    
    if (Firebase.RTDB.push(&fbdo, logPath, &json)) {
        Serial.println("[FB] ✅ Parcel event logged: " + event);
    } else {
        Serial.printf("[FB] ❌ Failed to log parcel event: %s\n", fbdo.errorReason().c_str());
    }
}

void FirebaseManager::commandStreamCallback(MultiPathStream stream) {
    if (globalFirebaseManager == nullptr) return;
    
    Serial.println("\n🔥 🔥 🔥 COMMAND STREAM UPDATE RECEIVED! 🔥 🔥 🔥");
    Serial.println("Path: " + stream.dataPath);
    Serial.println("Value: " + stream.value);

    // Handle lock control commands from app
    if (stream.get("/lock1")) {
        String command = stream.value;
        command.replace("\"", "");  // Remove JSON quotes
        Serial.println("🔓 Lock1 command: " + command);
        if (globalFirebaseManager->lockCommandCb) {
            globalFirebaseManager->lockCommandCb(1, command == "open");
        }
    }

    // Handle lock2 control
    if (stream.get("/lock2")) {
        String command = stream.value;
        command.replace("\"", "");
        Serial.println("🔓 Lock2 command: " + command);
        if (globalFirebaseManager->lockCommandCb) {
            globalFirebaseManager->lockCommandCb(2, command == "open");
        }
    }

    // Handle door control
    if (stream.get("/door1")) {
        String command = stream.value;
        command.replace("\"", "");
        Serial.println("🚪 Door1 command: " + command);
        // door control handled by hardware - reed switches auto-detect
    }

    // Handle emergency unlock command
    if (stream.get("/emergency_unlock")) {
        bool emergencyUnlock = (stream.value == "true");
        Serial.println("🚨 Emergency unlock: " + String(emergencyUnlock ? "ENABLED" : "DISABLED"));
        if (emergencyUnlock && globalFirebaseManager->emergencyCb) {
            globalFirebaseManager->emergencyCb();
        }
    }

    // Handle configuration changes
    if (stream.get("/config")) {
        Serial.println("⚙️ Configuration update received");
        // Process configuration changes
    }
}

void FirebaseManager::commandStreamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("[FB] ⏰ Command stream timed out, attempting to resume...");
    }
    
    if (globalFirebaseManager && !globalFirebaseManager->commandStream.httpConnected()) {
        Serial.printf("[FB] ❌ Command Stream Error - Code: %d, Reason: %s\n", 
                      globalFirebaseManager->commandStream.httpCode(), 
                      globalFirebaseManager->commandStream.errorReason().c_str());
        
        // Mark stream as inactive for reinitialization
        globalFirebaseManager->commandStreamActive = false;
    }
}

bool FirebaseManager::isReady() {
    return Firebase.ready();
}
