/*
 * ========================================
 * Smart Parcel Locker - ESP32-CAM (QR Scanner)
 * ========================================
 *
 * Hardware: ESP32-CAM (AI-Thinker)
 * Function: QR Code Scanner + ESP-NOW Transmitter
 * Sends scanned QR codes wirelessly to Main ESP32
 *
 * Libraries:
 * - ESP32QRCodeReader (by Alois Zingl)
 * - ESP32 Board Package
 */

#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include <esp_now.h>
#include <WiFi.h>

// ============================================================================
// CONFIGURATION - UPDATE THESE BEFORE DEPLOYMENT
// ============================================================================

// Main ESP32 MAC Address (Receiver)
// Get this by running: Serial.println(WiFi.macAddress()); on the main ESP32
uint8_t receiverMac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};  // <-- UPDATE THIS!

// Built-in LED for visual feedback
#define LED_PIN 33

// ============================================================================
// ESP-NOW DATA STRUCTURE
// ============================================================================
typedef struct __attribute__((packed)) {
    char qrData[32];        // QR code payload (parcelId)
    uint32_t timestamp;     // Scan timestamp (millis)
    uint8_t camMac[6];      // ESP32-CAM MAC address
} ESPNOW_QRPacket_t;

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
ESPNOW_QRPacket_t packet;

// Debounce control
unsigned long lastScanTime = 0;
const unsigned long SCAN_COOLDOWN_MS = 3000;  // 3 second debounce
char lastQrSent[32] = "";

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setupEspNow();
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void sendQrCode(const char* qrPayload);
void onQrCodeTask(void *pvParameters);
void blinkLed(int count, int duration);

// ============================================================================
// SETUP
// ============================================================================
void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("========================================");
    Serial.println("ParcelBox ESP32-CAM QR Scanner");
    Serial.println("========================================");

    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize QR Code Reader
    Serial.println("[SETUP] Initializing camera...");
    reader.setup();
    Serial.println("[SETUP] Camera ready");

    // Start QR detection on Core 1
    reader.beginOnCore(1);
    Serial.println("[SETUP] QR reader started on Core 1");

    // Initialize ESP-NOW
    Serial.println("[SETUP] Initializing ESP-NOW...");
    setupEspNow();

    // Success indication
    blinkLed(3, 100);
    Serial.println("[SETUP] Ready - scanning for QR codes");
    Serial.print("[SETUP] MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.println("========================================");

    // Start QR processing task
    xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

// ============================================================================
// LOOP (minimal - heavy lifting in task)
// ============================================================================
void loop()
{
    delay(5000);
    // Print heartbeat
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 30000) {
        lastHeartbeat = millis();
        Serial.println("[HEARTBEAT] UP " + String(millis() / 1000) + "s");
    }
}

// ============================================================================
// ESP-NOW SETUP
// ============================================================================
void setupEspNow()
{
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESPNOW] ERROR: Init failed");
        return;
    }

    // Register send callback
    esp_now_register_send_cb(onEspNowSent);

    // Add peer (main ESP32)
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;           // Use current channel (must match main ESP32)
    peerInfo.encrypt = false;       // No encryption for simplicity
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("[ESPNOW] ERROR: Failed to add peer");
        return;
    }

    Serial.println("[ESPNOW] Peer added (Main ESP32)");
}

// ============================================================================
// ESP-NOW SEND CALLBACK
// ============================================================================
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("[ESPNOW] Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAIL");

    // Visual feedback: 1 blink = success, 3 slow blinks = fail
    if (status == ESP_NOW_SEND_SUCCESS) {
        blinkLed(1, 100);
    } else {
        blinkLed(3, 300);
    }
}

// ============================================================================
// SEND QR CODE VIA ESP-NOW
// ============================================================================
void sendQrCode(const char* qrPayload)
{
    // Layer 1: Duplicate scan protection (CAM side)
    if (strcmp(qrPayload, lastQrSent) == 0 &&
        (millis() - lastScanTime) < SCAN_COOLDOWN_MS) {
        Serial.println("[DEBOUNCE] Skipping duplicate scan");
        return;
    }

    // Update debounce tracking
    strncpy(lastQrSent, qrPayload, 31);
    lastQrSent[31] = '\0';
    lastScanTime = millis();

    // Sanitize payload
    String qrClean = String(qrPayload);
    qrClean.trim();
    qrClean.replace("\r", "");
    qrClean.replace("\n", "");

    // Prepare packet
    memset(&packet, 0, sizeof(packet));
    strncpy(packet.qrData, qrClean.c_str(), 31);
    packet.qrData[31] = '\0';
    packet.timestamp = millis();
    WiFi.macAddress(packet.camMac);

    Serial.println("========================================");
    Serial.print("[QR] Detected: ");
    Serial.println(packet.qrData);
    Serial.print("[ESPNOW] Sending to: ");

    // Print MAC address
    for (int i = 0; i < 6; i++) {
        if (i > 0) Serial.print(":");
        Serial.printf("%02X", receiverMac[i]);
    }
    Serial.println();

    // Pre-send LED
    digitalWrite(LED_PIN, HIGH);
    delay(50);

    // Send via ESP-NOW
    esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&packet, sizeof(packet));

    digitalWrite(LED_PIN, LOW);

    Serial.print("[ESPNOW] Result: ");
    Serial.println(result == ESP_OK ? "OK" : "ERROR");
    Serial.println("========================================");
}

// ============================================================================
// QR CODE DETECTION TASK (FreeRTOS)
// ============================================================================
void onQrCodeTask(void *pvParameters)
{
    struct QRCodeData qrCodeData;

    while (true) {
        if (reader.receiveQrCode(&qrCodeData, 100)) {
            if (qrCodeData.valid) {
                const char* payload = (const char *)qrCodeData.payload;
                sendQrCode(payload);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// LED UTILITY
// ============================================================================
void blinkLed(int count, int duration)
{
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(duration);
        digitalWrite(LED_PIN, LOW);
        if (i < count - 1) delay(duration / 2);
    }
}
