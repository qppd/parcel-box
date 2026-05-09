#include "WiFiManagerCustom.h"

// ============================================================================
// WIFI MANAGER CUSTOM IMPLEMENTATION (ESP32-CAM)
// ============================================================================

WiFiManagerCustom::WiFiManagerCustom() : lastReconnectAttempt(0) {
}

bool WiFiManagerCustom::begin(const char* apName, const char* apPassword) {
    // Configure WiFiManager with timeouts
    wifiManager.setConfigPortalTimeout(DEFAULT_PORTAL_TIMEOUT);
    wifiManager.setConnectTimeout(DEFAULT_CONNECT_TIMEOUT);

    // Set static IP for captive portal
    wifiManager.setAPStaticIPConfig(
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0)
    );

    // Custom HTML styling for Parcel Box branding
    wifiManager.setCustomHeadElement(R"(
        <style>
            body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); font-family: Arial; }
            .container { background: white; border-radius: 10px; padding: 20px; max-width: 400px; margin: 50px auto; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
            h1 { color: #333; text-align: center; }
            p { color: #666; text-align: center; font-size: 14px; }
            input, button { width: 100%; padding: 12px; margin: 10px 0; border-radius: 5px; border: 1px solid #ddd; }
            input { font-size: 16px; }
            button { background: #667eea; color: white; border: none; cursor: pointer; font-weight: bold; }
            button:hover { background: #764ba2; }
        </style>
    )");

    if (!wifiManager.autoConnect(apName, apPassword)) {
        Serial.println("[WiFi] Failed to connect and hit timeout");
        return false;
    }

    Serial.println("[WiFi] Connected successfully!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());

    return true;
}

bool WiFiManagerCustom::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManagerCustom::getLocalIP() {
    return WiFi.localIP().toString();
}

void WiFiManagerCustom::reconnect() {
    if (!isConnected()) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = currentMillis;
            Serial.println("[WiFi] Attempting to reconnect...");
            WiFi.disconnect(false);
            WiFi.reconnect();
        }
    }
}

void WiFiManagerCustom::reset() {
    wifiManager.resetSettings();
    Serial.println("[WiFi] WiFi settings have been reset");
}

void WiFiManagerCustom::setConnectTimeout(uint16_t seconds) {
    wifiManager.setConnectTimeout(seconds);
}

void WiFiManagerCustom::setConfigPortalTimeout(uint16_t seconds) {
    wifiManager.setConfigPortalTimeout(seconds);
}