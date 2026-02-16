#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <Arduino.h>

// ============================================================================
// FIREBASE CONFIGURATION - Smart Parcel Locker
// ============================================================================
// This class centralizes Firebase Realtime Database credentials and URLs
// DO NOT COMMIT FirebaseConfig.cpp TO PUBLIC REPOSITORY
// Use FirebaseConfig.cpp.template for public distribution

class ParcelBoxFirebaseConfig {
public:
    // Get Firebase Host (RTDB domain)
    static const char* getFirebaseHost();
    
    // Get Database URL (complete HTTPS endpoint)
    static const char* getDatabaseURL();
    
    // Get API Key (from google-services.json)
    static const char* getApiKey();
    
    // Get Project ID
    static const char* getProjectId();
    
    // Firebase Realtime Database paths (structure)
    static const char* getParcelsDatabasePath();     // /parcels
    static const char* getDeviceStatusPath();        // /device_status
    static const char* getLocksStatusPath();         // /locks_status
    static const char* getHistoryPath();             // /history
    static const char* getConfigPath();              // /config
};

#endif // FIREBASE_CONFIG_H
