#ifndef WIFI_MANAGER_CUSTOM_H
#define WIFI_MANAGER_CUSTOM_H

#include <WiFi.h>
#include <WiFiManager.h>

// ============================================================================
// WIFI MANAGER CUSTOM - Smart Parcel Locker
// ============================================================================
// Handles WiFi connection setup with:
// - Automatic reconnection
// - Captive portal for credential entry
// - Custom HTML UI
// - Persistent credential storage

class WiFiManagerCustom {
public:
    // Constructor
    WiFiManagerCustom();
    
    // Initialize WiFi with captive portal
    // Returns: true if connected, false if timeout
    // apName: Access Point name shown in WiFi list
    // apPassword: Password for setup portal
    bool begin(const char* apName, const char* apPassword);
    
    // Check if WiFi is currently connected
    bool isConnected();
    
    // Get local IP address as string
    String getLocalIP();
    
    // Attempt to reconnect if disconnected
    void reconnect();
    
    // Reset all stored WiFi credentials
    void reset();
    
    // Set custom timeout for connection attempts (seconds)
    void setConnectTimeout(uint16_t seconds);
    
    // Set custom timeout for captive portal (seconds)
    void setConfigPortalTimeout(uint16_t seconds);

private:
    // WiFiManager instance
    WiFiManager wifiManager;
    
    // Track last reconnection attempt
    unsigned long lastReconnectAttempt;
    
    // Reconnection interval (5 seconds)
    static const unsigned long RECONNECT_INTERVAL = 5000;
    
    // Default timeouts
    static const uint16_t DEFAULT_CONNECT_TIMEOUT = 30;    // seconds
    static const uint16_t DEFAULT_PORTAL_TIMEOUT = 180;    // seconds (3 minutes)
};

#endif // WIFI_MANAGER_CUSTOM_H
