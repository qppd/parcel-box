#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Firebase_ESP_Client.h>
#include "FirebaseConfig.h"

// Forward declaration
class FirebaseManager;
extern FirebaseManager* globalFirebaseManager;

// Callback types for hardware control from Firebase commands
typedef void (*LockCommandCallback)(int lockNum, bool open);
typedef void (*EmergencyCallback)();

class FirebaseManager {
public:
    FirebaseManager();

    // Initialization
    void begin();
    void setDeviceId(const String& id);

    // Stream management
    void initializeStreams();
    void handleStreams();

    // Hardware control callback registration
    void setLockCommandCallback(LockCommandCallback cb);
    void setEmergencyCallback(EmergencyCallback cb);
    
    // Data writing
    void updateDeviceStatus(const String& deviceId, bool wifiConnected, bool fbConnected, float temperature, int humidity);
    void updateLockStatus(const String& deviceId, bool lock1Open, bool lock2Open, bool door1Open, bool door2Open);
    void logParcelEvent(const String& deviceId, const String& parcelId, const String& event);
    
    // Query methods
    bool isReady();
    
    // Public Firebase objects for global callback access
    FirebaseData fbdo;
    FirebaseData commandStream;
    
private:
    // Configuration
    FirebaseAuth auth;
    FirebaseConfig config;
    String deviceId;
    
    // Stream state
    bool commandStreamActive;
    unsigned long lastStatusUpdate;
    unsigned long lastStreamCheck;
    
    // Timing intervals
    static const unsigned long STATUS_UPDATE_INTERVAL = 5000;      // 5 seconds
    static const unsigned long STREAM_HEALTH_CHECK_INTERVAL = 30000; // 30 seconds
    
    // Internal methods
    void initializeCommandStream();
    
    // Registered hardware callbacks
    LockCommandCallback lockCommandCb = nullptr;
    EmergencyCallback emergencyCb = nullptr;

    // Callback functions (static for C-style callback compatibility)
    static void commandStreamCallback(MultiPathStream stream);
    static void commandStreamTimeoutCallback(bool timeout);
};

#endif // FIREBASE_MANAGER_H
