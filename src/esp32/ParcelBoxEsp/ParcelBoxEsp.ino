/*
 * ========================================
 * Smart Parcel Locker - ESP32 + Firebase
 * ========================================
 * 
 * Responsibilities:
 * 1. WiFi connectivity with captive portal setup
 * 2. Firebase Realtime Database integration
 * 3. QR code scanner interface via UART
 * 4. I2C LCD display for status feedback
 * 5. Real-time data streaming and persistence
 * 6. Send commands to Arduino Uno for hardware control
 * 
 * Key Features:
 * - Automatic WiFi reconnection
 * - Firebase data streaming (incoming parcel orders)
 * - Real-time lock status updates
 * - Parcel delivery history logging
 * - Device health monitoring
 * - Secure credential management
 * 
 * Hardware Connections:
 * - QR Scanner RX: GPIO16 (Serial2 RX)
 * - QR Scanner TX: GPIO17 (Serial2 TX)
 * - I2C LCD SDA: GPIO21 (I2C address: 0x27)
 * - I2C LCD SCL: GPIO22
 * - Arduino Uno RX: GPIO3 (Serial RX)
 * - Arduino Uno TX: GPIO1 (Serial TX)
 */

// ============================================================================
// INCLUDE SECTION
// ============================================================================
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <Preferences.h>

// Firebase library
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Custom configuration modules
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "WiFiManagerCustom.h"

// ============================================================================
// FIREBASE AND NETWORK OBJECTS
// ============================================================================
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiManagerCustom wifiManager;
Preferences preferences;

// ============================================================================
// I2C LCD INITIALIZATION
// ============================================================================
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// ============================================================================
// SYSTEM STATE STRUCTURE
// ============================================================================
struct SystemState {
  String device_id = "";
  String current_parcel_id = "";
  String current_qr_code = "";
  
  bool lock1_open = false;
  bool lock2_open = false;
  bool door1_status = false;
  bool door2_status = false;
  
  unsigned long last_scan_time = 0;
  unsigned long last_firebase_update = 0;
  unsigned long last_health_check = 0;
  
  bool wifi_connected = false;
  bool firebase_connected = false;
  bool valid_scan = false;
} system_state;

// ============================================================================
// FIREBASE CONNECTION STATE
// ============================================================================
bool firebaseInitialized = false;
bool listeningToParcels = false;

// ============================================================================
// TIMING INTERVALS
// ============================================================================
const unsigned long FIREBASE_UPDATE_INTERVAL = 5000;      // 5 seconds
const unsigned long HEALTH_CHECK_INTERVAL = 30000;        // 30 seconds
const unsigned long RECONNECT_CHECK_INTERVAL = 10000;     // 10 seconds
const unsigned long QR_SCAN_TIMEOUT = 30000;              // 30 seconds

// State tracking for intervals
unsigned long lastFirebaseUpdate = 0;
unsigned long lastHealthCheck = 0;
unsigned long lastReconnectCheck = 0;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setup();
void loop();
void setupWiFi();
void setupI2C_LCD();
void initializeFirebase();
void displayLCD(String line1, String line2 = "", String line3 = "", String line4 = "");
void sendCommandToArduino(String command);
void processArduinoResponse();
void handleParcelScanned(String qr_code);
void validateAndOpenLocks(String qr_code);
void updateFirebaseStatus();
void logParcelHistory(String parcel_id, String event);
void processQRScanner();
void checkSystemHealth();
void debugPrint(String msg);
void tokenStatusCallback(TokenInfo info);
void generateDeviceId();
void checkWiFiConnection();
void reconnectWiFi();
void initializeNTP();

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(BAUD_ARDUINO);
  
  // Initialize Serial2 for QR Scanner
  Serial2.begin(BAUD_QR_SCANNER, SERIAL_8N1, QR_SCANNER_RX_PIN, QR_SCANNER_TX_PIN);
  
  // Initialize I2C and LCD
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  setupI2C_LCD();
  
  // Display startup message
  displayLCD("PARCEL LOCKER", "Initializing...", "v1.0", "");
  delay(1000);
  
  debugPrint("================================================");
  debugPrint("Smart Parcel Locker - ESP32 Startup");
  debugPrint("================================================");
  
  // Generate unique device ID
  generateDeviceId();
  debugPrint("Device ID: " + system_state.device_id);
  
  // Setup WiFi with captive portal
  displayLCD("Setting up WiFi", "Connect to portal", "ParcelBox_Setup", "");
  setupWiFi();
  
  // Initialize Firebase Realtime Database
  if (system_state.wifi_connected) {
    displayLCD("Initializing", "Firebase...", "", "");
    initializeFirebase();
  }
  
  // Display ready state
  displayLCD("SYSTEM READY", "Waiting for parcel", "WiFi: Connected", "FB: " + String(system_state.firebase_connected ? "OK" : "Retry"));
  
  debugPrint("Setup complete!");
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Check WiFi connection and maintain it
  checkWiFiConnection();
  
  // Process Firebase connection
  if (system_state.wifi_connected && fbdo.httpConnected()) {
    system_state.firebase_connected = true;
  } else {
    system_state.firebase_connected = false;
  }
  
  // Process QR code scanner input
  processQRScanner();
  
  // Process responses from Arduino
  processArduinoResponse();
  
  // Update Firebase status at regular intervals
  if (millis() - lastFirebaseUpdate > FIREBASE_UPDATE_INTERVAL) {
    lastFirebaseUpdate = millis();
    if (system_state.firebase_connected) {
      updateFirebaseStatus();
    }
  }
  
  // System health check
  if (millis() - lastHealthCheck > HEALTH_CHECK_INTERVAL) {
    lastHealthCheck = millis();
    checkSystemHealth();
  }
  
  delay(100);
}

// ============================================================================
// WIFI SETUP AND MANAGEMENT
// ============================================================================
void setupWiFi() {
  debugPrint("Initializing WiFi Manager...");
  
  // Begin WiFi setup with captive portal
  if (!wifiManager.begin("ParcelBox_Setup", "password123")) {
    debugPrint("WiFi setup timeout - using previous credentials or offline mode");
    displayLCD("WiFi Timeout", "Will retry in", "offline mode", "");
    system_state.wifi_connected = false;
    return;
  }
  
  debugPrint("WiFi Connected!");
  debugPrint("IP Address: " + wifiManager.getLocalIP());
  
  displayLCD("WiFi Connected", "IP: " + wifiManager.getLocalIP(), "", "");
  system_state.wifi_connected = true;
  delay(2000);
  
  // Initialize NTP for time synchronization
  initializeNTP();
}

void initializeNTP() {
  debugPrint("Synchronizing time with NTP...");
  
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 20) {
    delay(500);
    now = time(nullptr);
    attempts++;
  }
  
  struct tm timeinfo = *localtime(&now);
  debugPrint("Time synchronized: " + String(asctime(&timeinfo)));
}

void checkWiFiConnection() {
  unsigned long currentMillis = millis();
  
  // Check connection status periodically
  if (currentMillis - lastReconnectCheck > RECONNECT_CHECK_INTERVAL) {
    lastReconnectCheck = currentMillis;
    
    bool connected = wifiManager.isConnected();
    
    if (!connected && system_state.wifi_connected) {
      debugPrint("WiFi connection lost!");
      system_state.wifi_connected = false;
      displayLCD("WiFi Disconnected", "Reconnecting...", "", "");
    }
    
    if (!connected) {
      wifiManager.reconnect();
    } else if (!system_state.wifi_connected) {
      debugPrint("WiFi reconnected!");
      system_state.wifi_connected = true;
      if (!firebaseInitialized) {
        initializeFirebase();
      }
    }
  }
}

void reconnectWiFi() {
  wifiManager.reset();
  delay(1000);
  setupWiFi();
}

// ============================================================================
// I2C LCD SETUP AND DISPLAY
// ============================================================================
void setupI2C_LCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  debugPrint("LCD Initialized");
}

void displayLCD(String line1, String line2, String line3, String line4) {
  lcd.clear();
  
  // Display line 1 (truncate to 20 chars)
  if (line1.length() > 0) {
    line1 = line1.substring(0, LCD_COLS);
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  
  // Display line 2
  if (line2.length() > 0) {
    line2 = line2.substring(0, LCD_COLS);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  
  // Display line 3
  if (line3.length() > 0) {
    line3 = line3.substring(0, LCD_COLS);
    lcd.setCursor(0, 2);
    lcd.print(line3);
  }
  
  // Display line 4
  if (line4.length() > 0) {
    line4 = line4.substring(0, LCD_COLS);
    lcd.setCursor(0, 3);
    lcd.print(line4);
  }
}

// ============================================================================
// FIREBASE INITIALIZATION AND SETUP
// ============================================================================
void initializeFirebase() {
  if (firebaseInitialized) {
    return;
  }
  
  debugPrint("Initializing Firebase Realtime Database...");
  
  // Assign the API host and database URL
  config.host = ParcelBoxFirebaseConfig::getFirebaseHost();
  config.database_url = ParcelBoxFirebaseConfig::getDatabaseURL();
  config.api_key = ParcelBoxFirebaseConfig::getApiKey();
  config.token_status_callback = tokenStatusCallback;
  config.max_retries = 2;
  config.tcp_data_timeout_sec = 5;
  config.ssl_data_timeout_ms = 5000;
  config.wifi_reconnect_timeout_ms = 10000;
  config.firebase_stream_class = None;
  
  // Anonymous authentication (requires Firebase auth settings)
  // If using email/password or custom auth, modify here
  auth.user.email = "";
  auth.user.password = "";
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Register device in Firebase
  registerDeviceInFirebase();
  
  debugPrint("Firebase initialized successfully!");
  firebaseInitialized = true;
}

void registerDeviceInFirebase() {
  String devicePath = ParcelBoxFirebaseConfig::getDeviceStatusPath();
  
  fbdo.setBSSLBufferSize(2048, 1024);
  fbdo.setResponseSize(2048);
  
  // Create device status object
  FirebaseJson json;
  json.set("device_id", system_state.device_id);
  json.set("model", "ParcelBox_ESP32");
  json.set("version", "1.0.0");
  json.set("wifi_connected", true);
  json.set("firebase_connected", true);
  json.set("last_heartbeat", millis());
  
  // Push to Firebase
  if (Firebase.RTDB.setJSON(&fbdo, devicePath.c_str(), &json)) {
    debugPrint("Device registered in Firebase");
  } else {
    debugPrint("Failed to register device: " + String(fbdo.errorReason()));
  }
}

// ============================================================================
// FIREBASE TOKEN STATUS CALLBACK
// ============================================================================
void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    debugPrint("Firebase Token Error: " + String(info.error.message));
  } else if (info.status == token_status_ready) {
    debugPrint("Firebase Token Ready");
  } else if (info.status == token_status_expired) {
    debugPrint("Firebase Token Expired - Refreshing");
  }
}

// ============================================================================
// UPDATE FIREBASE STATUS
// ============================================================================
void updateFirebaseStatus() {
  if (!system_state.firebase_connected) {
    return;
  }
  
  String devicePath = ParcelBoxFirebaseConfig::getDeviceStatusPath();
  
  // Update device heartbeat
  if (Firebase.RTDB.setInt(&fbdo, (devicePath + "/last_heartbeat").c_str(), millis())) {
    // Successfully updated
  }
  
  // Update lock status
  String locksPath = ParcelBoxFirebaseConfig::getLocksStatusPath();
  
  FirebaseJson locksJson;
  locksJson.set("lock1/status", system_state.lock1_open ? "open" : "closed");
  locksJson.set("lock1/last_update", millis());
  locksJson.set("lock2/status", system_state.lock2_open ? "open" : "closed");
  locksJson.set("lock2/last_update", millis());
  
  Firebase.RTDB.setJSON(&fbdo, locksPath.c_str(), &locksJson);
}

// ============================================================================
// PARCEL HISTORY LOGGING
// ============================================================================
void logParcelHistory(String parcel_id, String event) {
  if (!system_state.firebase_connected) {
    debugPrint("Firebase not connected - skipping history log");
    return;
  }
  
  String historyPath = ParcelBoxFirebaseConfig::getHistoryPath();
  unsigned long timestamp = millis();
  
  FirebaseJson historyEntry;
  historyEntry.set("parcel_id", parcel_id);
  historyEntry.set("event", event);
  historyEntry.set("timestamp", timestamp);
  historyEntry.set("device_id", system_state.device_id);
  
  // Push entry to history (creates new child with auto-ID)
  if (Firebase.RTDB.pushJSON(&fbdo, historyPath.c_str(), &historyEntry)) {
    debugPrint("History logged: " + parcel_id + " -> " + event);
  } else {
    debugPrint("Failed to log history: " + String(fbdo.errorReason()));
  }
}

// ============================================================================
// QR CODE SCANNER PROCESSING
// ============================================================================
void processQRScanner() {
  if (Serial2.available()) {
    String qr_code = Serial2.readStringUntil('\n');
    qr_code.trim();  // Remove whitespace
    
    if (qr_code.length() > 0) {
      debugPrint("QR Code Scanned: " + qr_code);
      handleParcelScanned(qr_code);
    }
  }
}

// ============================================================================
// PARCEL HANDLING WORKFLOW
// ============================================================================
void handleParcelScanned(String qr_code) {
  system_state.current_qr_code = qr_code;
  system_state.last_scan_time = millis();
  
  // Display scanning feedback
  displayLCD("QR SCANNED", qr_code, "Validating...", "");
  
  // Log scan event
  logParcelHistory(qr_code, "QR_SCANNED");
  
  // Validate QR code with backend
  validateAndOpenLocks(qr_code);
}

void validateAndOpenLocks(String qr_code) {
  debugPrint("Validating QR: " + qr_code);
  
  // Simulate validation - replace with actual Firebase lookup
  bool is_valid = false;
  
  // Check Firebase for parcel information
  if (system_state.firebase_connected) {
    String parcelPath = String(ParcelBoxFirebaseConfig::getParcelsDatabasePath()) + "/" + qr_code;
    
    if (Firebase.RTDB.getJSON(&fbdo, parcelPath.c_str())) {
      if (fbdo.dataType() == "json") {
        FirebaseJson &json = fbdo.jsonObject();
        is_valid = true;
        system_state.current_parcel_id = qr_code;
        
        debugPrint("Parcel found in Firebase");
        logParcelHistory(qr_code, "PARCEL_FOUND");
      }
    }
  } else {
    // Fallback: accept if QR is long enough for offline mode
    if (qr_code.length() >= 5) {
      is_valid = true;
      system_state.current_parcel_id = qr_code;
    }
  }
  
  if (is_valid) {
    debugPrint("QR Validation: SUCCESS");
    system_state.valid_scan = true;
    
    displayLCD("Access Granted", "Opening locks...", "", "");
    logParcelHistory(qr_code, "VALIDATION_SUCCESS");
    
    // Open parcel door (Lock #1)
    sendCommandToArduino("AT+LOCK1,OPEN");
    delay(LOCK_OPERATION_DELAY);
    system_state.lock1_open = true;
    
    // Open payment box (Lock #2)
    sendCommandToArduino("AT+LOCK2,OPEN");
    delay(LOCK_OPERATION_DELAY);
    system_state.lock2_open = true;
    
    // Success feedback
    sendCommandToArduino("AT+BUZZ,SUCCESS");
    displayLCD("DOORS OPEN", "Place parcel in box", "Complete payment", "Door closes auto");
    
    // Update Firebase
    updateFirebaseStatus();
    
  } else {
    debugPrint("QR Validation: FAILED");
    system_state.valid_scan = false;
    
    // Access denied
    sendCommandToArduino("AT+BUZZ,ALERT");
    displayLCD("Access Denied", "Invalid QR Code", "Try again", "");
    
    logParcelHistory(qr_code, "VALIDATION_FAILED");
    delay(3000);
    displayLCD("READY", "Scan parcel QR", "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"), 
               "FB: " + String(system_state.firebase_connected ? "OK" : "---"));
  }
}

// ============================================================================
// ARDUINO COMMUNICATION
// ============================================================================
void sendCommandToArduino(String command) {
  debugPrint("→ Arduino: " + command);
  Serial.println(command);
}

void processArduinoResponse() {
  if (Serial.available()) {
    String response = Serial.readStringUntil('\n');
    response.trim();
    
    debugPrint("← Arduino: " + response);
    
    // Handle events from Arduino
    if (response.startsWith("EVENT:")) {
      String event = response.substring(6);
      
      if (event == "DOOR1_CLOSED") {
        system_state.door1_status = false;
        system_state.lock1_open = false;
        
        debugPrint("Parcel door closed - marking as delivered");
        displayLCD("Parcel Locked", "Delivery complete", "", "");
        
        // Log event
        logParcelHistory(system_state.current_parcel_id, "PARCEL_DELIVERED");
        
        // Update Firebase parcel status
        if (system_state.firebase_connected && system_state.current_parcel_id.length() > 0) {
          String parcelPath = String(ParcelBoxFirebaseConfig::getParcelsDatabasePath()) + "/" + system_state.current_parcel_id + "/status";
          Firebase.RTDB.setString(&fbdo, parcelPath.c_str(), "delivered");
        }
        
        delay(2000);
        displayLCD("READY", "Scan parcel QR", "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"), 
                   "FB: " + String(system_state.firebase_connected ? "OK" : "---"));
        
        // Reset for next parcel
        system_state.current_parcel_id = "";
        system_state.current_qr_code = "";
      }
      else if (event == "DOOR2_CLOSED") {
        system_state.door2_status = false;
        system_state.lock2_open = false;
        debugPrint("Payment box closed");
      }
      else if (event.startsWith("ERROR:")) {
        debugPrint("Arduino Error: " + event);
        logParcelHistory(system_state.current_parcel_id, "ERROR: " + event);
      }
    }
  }
}

// ============================================================================
// SYSTEM HEALTH CHECK
// ============================================================================
void checkSystemHealth() {
  debugPrint("\n=== System Health Check ===");
  debugPrint("Uptime: " + String(millis() / 1000) + "s");
  debugPrint("WiFi Status: " + String(system_state.wifi_connected ? "Connected" : "Disconnected"));
  debugPrint("Firebase Status: " + String(system_state.firebase_connected ? "Connected" : "Disconnected"));
  debugPrint("Parcel ID: " + (system_state.current_parcel_id.length() > 0 ? system_state.current_parcel_id : "None"));
  debugPrint("Lock 1: " + String(system_state.lock1_open ? "OPEN" : "CLOSED"));
  debugPrint("Lock 2: " + String(system_state.lock2_open ? "OPEN" : "CLOSED"));
  debugPrint("===========================\n");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================
void generateDeviceId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  system_state.device_id = "PARCELBOX_" + String(macStr);
}

void debugPrint(String msg) {
  Serial.print("[ESP32] ");
  Serial.println(msg);
}

// ============================================================================
// ADDITIONAL CONTROL FUNCTIONS
// ============================================================================
void closeLocksAfterDelivery() {
  debugPrint("Closing locks after delivery...");
  sendCommandToArduino("AT+LOCK1,CLOSE");
  delay(LOCK_OPERATION_DELAY);
  sendCommandToArduino("AT+LOCK2,CLOSE");
  
  system_state.lock1_open = false;
  system_state.lock2_open = false;
}

void emergencyLockdown() {
  debugPrint("EMERGENCY LOCKDOWN ACTIVATED!");
  displayLCD("LOCKDOWN", "System Secured", "Contact Admin", "");
  
  sendCommandToArduino("AT+LOCK1,CLOSE");
  delay(LOCK_OPERATION_DELAY);
  sendCommandToArduino("AT+LOCK2,CLOSE");
  sendCommandToArduino("AT+BUZZ,ALERT");
  
  logParcelHistory("SYSTEM", "EMERGENCY_LOCKDOWN");
  
  system_state.lock1_open = false;
  system_state.lock2_open = false;
}

void resetSystem() {
  system_state.current_parcel_id = "";
  system_state.current_qr_code = "";
  system_state.lock1_open = false;
  system_state.lock2_open = false;
  system_state.valid_scan = false;
  
  closeLocksAfterDelivery();
  displayLCD("SYSTEM RESET", "Ready for next parcel", "", "");
}
