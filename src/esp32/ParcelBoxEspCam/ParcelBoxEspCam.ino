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
#include <esp_wifi.h>
#include "WiFiManagerCustom.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// Main ESP32 MAC Address (Receiver)
uint8_t receiverMac[] = {0xB0, 0xCB, 0xD8, 0x03, 0xD6, 0xA4};  // B0:CB:D8:03:D6:A4

// Built-in LED for visual feedback
#define LED_PIN 33

// WiFi channel lock — MUST match Main ESP32's WiFi channel
// Find main ESP32 channel with: Serial.println(WiFi.channel());
#define ESPNOW_FIXED_CHANNEL 1  // Set this to your main ESP32's WiFi channel

// ============================================================================
// ESP-NOW QR PACKET STRUCTURE — MUST match ESPNOW_CONFIG.h on Main ESP32
// ============================================================================
typedef struct __attribute__((packed)) {
  char qrData[32];        // QR payload (parcelId)
  uint32_t timestamp;     // Scan timestamp (millis)
  uint8_t camMac[6];      // ESP32-CAM MAC address
} ESPNOW_QRPacket_t;

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
ESPNOW_QRPacket_t packet;
WiFiManagerCustom wifiManager;

// Debounce control
unsigned long lastScanTime = 0;
const unsigned long SCAN_COOLDOWN_MS = 3000;  // 3 second debounce
char lastQrSent[32] = "";

// Last scan result from QR reader (persisted across task calls)
String lastQrPayload = "";
unsigned long lastInvalidPrint = 0;
const unsigned long INVALID_PRINT_COOLDOWN = 5000;  // Only print [QR] Invalid once per 5s

// ESP-NOW peer handle
esp_now_peer_info_t peerInfo;

// ACK tracking — the Main ESP32 can send back an EspNowMessage ACK
// For simplicity, we use send callback as ACK confirmation
bool lastSendSucceeded = true;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setupEspNow();
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onEspNowSent(const wifi_tx_info_t *wifi_tx_info, esp_now_send_status_t status);
#else
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
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
    Serial.println("ParcelBox ESP32-CAM QR Scanner v2.0");
    Serial.println("========================================");

    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Check PSRAM
    if (psramFound()) {
        Serial.printf("[SETUP] PSRAM: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("[SETUP] WARNING: No PSRAM! Camera will not work.");
        Serial.println("[SETUP] Enable PSRAM in Tools -> Board Settings");
    }

    // Initialize QR Code Reader
    Serial.println(F("[SETUP] Initializing camera..."));
    if (reader.setup()) {
        Serial.println(F("[SETUP] Camera OK"));
    } else {
        Serial.println(F("[SETUP] ERROR: Camera init FAILED"));
    }

    reader.beginOnCore(1);
    Serial.println(F("[SETUP] QR reader on Core 1"));

    // Connect to WiFi
    Serial.println(F("[SETUP] Starting WiFi..."));
    if (wifiManager.begin("ParcelBoxCam_Setup", "password123")) {
        Serial.print("[SETUP] WiFi OK, ch: ");
        Serial.println(WiFi.channel());
    } else {
        Serial.println(F("[SETUP] WiFi FAILED"));
    }

    // Initialize ESP-NOW
    Serial.println(F("[SETUP] Initializing ESP-NOW..."));
    setupEspNow();

    // Success indication
    blinkLed(3, 100);
    Serial.println(F("[SETUP] Ready - scanning for QR codes"));
    Serial.print("[SETUP] MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.println("========================================");

    // Start QR processing task
    xTaskCreate(onQrCodeTask, "onQrCode", 6 * 1024, NULL, 4, NULL);
}

// ============================================================================
// LOOP (non-blocking)
// ============================================================================
void loop()
{
    // Maintain WiFi connection — reconnects when dropped
    wifiManager.reconnect();

    // Print heartbeat every 60s
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 60000) {
        lastHeartbeat = millis();
        Serial.print("[HEARTBEAT] UP ");
        Serial.print(millis() / 1000);
        Serial.println("s");
    }

    // No delay! WiFi handling and task scheduler run freely.
    delay(10);  // Small yield for watchdog
}

// ============================================================================
// ESP-NOW SETUP
// ============================================================================
void setupEspNow()
{
    // Deinit first (safe for reinit)
    esp_now_deinit();

    WiFi.mode(WIFI_STA);

    // CRITICAL: Lock WiFi channel to match Main ESP32
    // ESP-NOW only works when both devices are on the same channel
    wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL };
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(ESPNOW_FIXED_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    Serial.printf("[ESPNOW] Forced to channel %d\n", ESPNOW_FIXED_CHANNEL);

    if (esp_now_init() != ESP_OK) {
        Serial.println(F("[ESPNOW] Init FAILED"));
        return;
    }

    // Register send callback
    esp_now_register_send_cb(onEspNowSent);

    // Register receive callback (for potential ACKs from main ESP32)
    // esp_now_register_recv_cb(onEspNowReceived);  // Uncomment if ACK handling needed

    // Add peer — Main ESP32
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = 0;  // 0 = use current (locked) channel
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println(F("[ESPNOW] Peer add FAILED"));
        return;
    }

    Serial.print(F("[ESPNOW] Peer added (Main ESP32)"));
    Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X\n",
        receiverMac[0], receiverMac[1], receiverMac[2],
        receiverMac[3], receiverMac[4], receiverMac[5]);
}

// ============================================================================
// ESP-NOW SEND CALLBACK
// ============================================================================
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onEspNowSent(const wifi_tx_info_t *wifi_tx_info, esp_now_send_status_t status)
#else
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        if (!lastSendSucceeded) {
            // Only print on state change from fail to success
            Serial.println(F("[ESPNOW] ACK RECEIVED"));
            lastSendSucceeded = true;
        }
        blinkLed(1, 100);
    } else {
        if (lastSendSucceeded) {
            Serial.println(F("[ESPNOW] Send FAILED"));
            lastSendSucceeded = false;
        }
        blinkLed(3, 300);
    }
}

// ============================================================================
// SEND QR CODE VIA ESP-NOW
// ============================================================================
void sendQrCode(const char* qrPayload)
{
    // Layer 1: Duplicate scan (CAM side)
    if (strcmp(qrPayload, lastQrSent) == 0 &&
        (millis() - lastScanTime) < SCAN_COOLDOWN_MS) {
        return;  // Silent — don't print debounce noise
    }

    // Update debounce
    strncpy(lastQrSent, qrPayload, 31);
    lastQrSent[31] = '\0';
    lastScanTime = millis();

    // Sanitize payload
    String qrClean = String(qrPayload);
    qrClean.trim();
    qrClean.replace("\r", "");
    qrClean.replace("\n", "");

    if (qrClean.length() == 0) {
        return;  // Don't send empty QR
    }

    // Prepare packet
    memset(&packet, 0, sizeof(packet));
    strncpy(packet.qrData, qrClean.c_str(), 31);
    packet.qrData[31] = '\0';
    packet.timestamp = millis();
    WiFi.macAddress(packet.camMac);

    // Clean output
    Serial.print("[QR] Payload: ");
    Serial.println(packet.qrData);

    // Pre-send LED
    digitalWrite(LED_PIN, HIGH);

    // Send via ESP-NOW
    esp_err_t result = esp_now_send(receiverMac, (uint8_t *)&packet, sizeof(packet));

    digitalWrite(LED_PIN, LOW);

    if (result != ESP_OK) {
        Serial.println(F("[ESPNOW] Send queue FAILED"));
    }
}

// ============================================================================
// QR CODE DETECTION TASK (FreeRTOS)
// ============================================================================
void onQrCodeTask(void *pvParameters)
{
    struct QRCodeData qrCodeData;
    unsigned long cooldownUntil = 0;

    while (true) {
        if (reader.receiveQrCode(&qrCodeData, 100)) {
            if (qrCodeData.valid) {
                const char* payload = (const char *)qrCodeData.payload;
                String payloadStr = String(payload);
                payloadStr.trim();

                // Minimal validation — skip empty/trash reads
                if (payloadStr.length() < 3) {
                    // Too short to be valid — silently ignore
                    // Print only once per 5s to avoid flooding
                    if (millis() - lastInvalidPrint > INVALID_PRINT_COOLDOWN) {
                        Serial.println(F("[QR] Invalid QR code data"));
                        lastInvalidPrint = millis();
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    continue;
                }

                // Check cooldown to avoid re-scanning the same QR too fast
                if (millis() < cooldownUntil) {
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    continue;
                }

                sendQrCode(payload);
                cooldownUntil = millis() + 2000;  // 2s min cooldown between sends
            } else {
                if (millis() - lastInvalidPrint > INVALID_PRINT_COOLDOWN) {
                    Serial.println(F("[QR] Invalid QR code data"));
                    lastInvalidPrint = millis();
                }
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