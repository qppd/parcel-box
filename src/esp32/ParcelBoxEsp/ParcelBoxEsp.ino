/*
 * ========================================
 * Smart Parcel Locker - ESP32 (38-pin)
 * ========================================
 *
 * Single-MCU design: ESP32 controls ALL hardware directly.
 * Migrated from dual-MCU (ESP32 + Arduino Uno) architecture.
 *
 * Responsibilities:
 * 1. WiFi connectivity with captive portal setup
 * 2. Firebase Realtime Database integration
 * 3. QR code scanner interface via UART1
 * 4. I2C LCD display for status feedback
 * 5. Direct relay control for solenoid locks
 * 6. Reed switch door sensor monitoring
 * 7. Piezo buzzer feedback (LEDC PWM)
 * 8. SIM800L GSM communication via UART2
 * 9. Serial command testing system (USB)
 *
 * Hardware Connections (ESP32 38-pin):
 * - Relay 1 (Lock 1):   GPIO 34
 * - Relay 2 (Lock 2):   GPIO 35
 * - SIM800L RX:          GPIO 16 (UART2)
 * - SIM800L TX:          GPIO 17 (UART2)
 * - SIM800L RST:         GPIO 5
 * - Reed Switch 1:       GPIO 12
 * - Reed Switch 2:       GPIO 14
 * - Buzzer:              GPIO 23
 * - I2C LCD SDA:         GPIO 21
 * - I2C LCD SCL:         GPIO 22
 * - QR Scanner RX:       GPIO 33 (UART1)
 * - QR Scanner TX:       GPIO 26 (UART1)
 * - QR Scanner RST:      GPIO 25
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
// TokenHelper not needed — using legacy_token (Database Secret) auth
#include <addons/RTDBHelper.h>

// Custom configuration modules
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "WiFiManagerCustom.h"

// ============================================================================
// HARDWARE SERIAL PORTS
// ============================================================================
HardwareSerial sim800l(2);    // UART2 for SIM800L (GPIO16 RX / GPIO17 TX)
HardwareSerial qrScanner(1);  // UART1 for QR Scanner (GPIO33 RX / GPIO26 TX)

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
  bool door1_open = false;
  bool door2_open = false;

  unsigned long last_scan_time = 0;
  unsigned long last_firebase_update = 0;
  unsigned long last_health_check = 0;
  unsigned long last_sms_time = 0;

  bool wifi_connected = false;
  bool firebase_connected = false;
  bool valid_scan = false;
} system_state;

// ============================================================================
// FIREBASE CONNECTION STATE
// ============================================================================
bool firebaseInitialized = false;

// ============================================================================
// SERIAL MONITOR FLAGS (toggled via test commands)
// ============================================================================
bool reed1_monitor = false;        // Continuous reed switch 1 printing
bool reed2_monitor = false;        // Continuous reed switch 2 printing
bool qr_monitor = false;           // Print raw QR scanner data to Serial
bool gsm_monitor = false;          // Forward all GSM responses to Serial
unsigned long gsmRespTimeout = 0;  // End time for AT command response window
unsigned long lastReedPrint = 0;

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

// WiFi & Firebase
void setupWiFi();
void initializeNTP();
void checkWiFiConnection();
void reconnectWiFi();
void initializeFirebase();
void registerDeviceInFirebase();
void updateFirebaseStatus();
void logParcelHistory(String parcel_id, String event);

// LCD
void setupI2C_LCD();
void displayLCD(String line1, String line2 = "", String line3 = "", String line4 = "");

// Hardware Control
void openLock(int lockNum);
void closeLock(int lockNum);
void playBuzzer(String tone_type);
void checkDoorSensors();
void handleDoorClosed(int doorNum);
void initSIM800L();
void resetSIM800L();
void sendSMS(String phone, String message);

// QR & Parcel Workflow
void processQRScanner();
void processGSM();
void handleParcelScanned(String qr_code);
void validateAndOpenLocks(String qr_code);
void closeLocksAfterDelivery();
void emergencyLockdown();
void resetSystem();

// Serial Testing
void processSerialCommands();
void printHelp();

// Utility
void generateDeviceId();
void checkSystemHealth();
void debugPrint(String msg);

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
  // Initialize USB Serial for debug + test commands
  Serial.begin(BAUD_SERIAL);
  Serial.setTimeout(50);   // Short timeout — readStringUntil() returns within 50ms
  delay(500);

  debugPrint("================================================");
  debugPrint("Smart Parcel Locker - ESP32 Startup");
  debugPrint("================================================");

  // ── Step 1: GPIO ──────────────────────────────────────────────────────────
  Serial.println("[SETUP 1/7] Initializing GPIO pins..."); Serial.flush();
  //pinMode(RELAY_1_PIN, OUTPUT);
  //pinMode(RELAY_2_PIN, OUTPUT);
  //pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DOOR_SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(DOOR_SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(SIM800L_RST_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(SIM800L_RST_PIN, HIGH);
  ledcAttach(BUZZER_PIN, 1000, BUZZER_RESOLUTION);
  Serial.println("[SETUP 1/7] GPIO OK"); Serial.flush();

  // ── Step 2: UARTs ─────────────────────────────────────────────────────────
  Serial.println("[SETUP 2/7] Initializing UARTs..."); Serial.flush();
  Serial.println("[SETUP 2/7]   SIM800L  -> UART2 RX=16 TX=17 @9600"); Serial.flush();
  sim800l.begin(BAUD_SIM800L, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  sim800l.setTimeout(50);
  Serial.println("[SETUP 2/7]   QR Scanner -> UART1 RX=33 TX=26 RST=25 @9600"); Serial.flush();
  pinMode(QR_SCANNER_RST_PIN, OUTPUT);
  digitalWrite(QR_SCANNER_RST_PIN, HIGH);
  delay(200);
  qrScanner.begin(BAUD_QR_SCANNER, SERIAL_8N1, QR_SCANNER_RX_PIN, QR_SCANNER_TX_PIN);
  qrScanner.setTimeout(100);
  Serial.println("[SETUP 2/7] UARTs OK"); Serial.flush();

  // ── Step 3: I2C + LCD ─────────────────────────────────────────────────────
  Serial.println("[SETUP 3/7] Initializing I2C + LCD (addr=0x27)..."); Serial.flush();
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  setupI2C_LCD();
  displayLCD("PARCEL LOCKER", "Initializing...", "v2.0 (ESP32)", "");
  delay(1000);
  Serial.println("[SETUP 3/7] LCD OK"); Serial.flush();

  // ── Step 4: Device ID ─────────────────────────────────────────────────────
  Serial.println("[SETUP 4/7] Generating Device ID..."); Serial.flush();
  generateDeviceId();
  Serial.println("[SETUP 4/7] Device ID: " + system_state.device_id); Serial.flush();

  // ── Step 5: SIM800L ───────────────────────────────────────────────────────
  Serial.println("[SETUP 5/7] Initializing SIM800L (takes ~5 sec)..."); Serial.flush();
  initSIM800L();
  Serial.println("[SETUP 5/7] SIM800L OK"); Serial.flush();
  playBuzzer("startup");

  // ── Step 6: WiFi ──────────────────────────────────────────────────────────
  Serial.println("[SETUP 6/7] Starting WiFi..."); Serial.flush();
  Serial.println("[SETUP 6/7]   If no saved WiFi: join 'ParcelBox_Setup' (pw: password123)"); Serial.flush();
  Serial.println("[SETUP 6/7]   Then open browser -> 192.168.4.1 to configure."); Serial.flush();
  Serial.println("[SETUP 6/7]   Auto-timeout in 180 sec if no action."); Serial.flush();
  displayLCD("Setting up WiFi", "Join: ParcelBox", "_Setup or wait...", "pw: password123");
  setupWiFi();
  Serial.println("[SETUP 6/7] WiFi result: " + String(system_state.wifi_connected ? "CONNECTED" : "OFFLINE")); Serial.flush();

  // ── Step 7: Firebase ──────────────────────────────────────────────────────
  if (system_state.wifi_connected) {
    Serial.println("[SETUP 7/7] Initializing Firebase..."); Serial.flush();
    displayLCD("Initializing", "Firebase...", "", "");
    initializeFirebase();
    Serial.println("[SETUP 7/7] Firebase result: " + String(system_state.firebase_connected ? "CONNECTED" : "FAILED")); Serial.flush();
  } else {
    Serial.println("[SETUP 7/7] Firebase SKIPPED (no WiFi)"); Serial.flush();
  }

  // ── Ready ─────────────────────────────────────────────────────────────────
  displayLCD("SYSTEM READY", "Waiting for parcel",
             "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"),
             "FB: " + String(system_state.firebase_connected ? "OK" : "---"));

  Serial.println();
  Serial.println("============================================");
  Serial.println("  PARCEL BOX READY - Type 'help' for cmds");
  Serial.println("  WiFi: " + String(system_state.wifi_connected ? "Connected" : "Offline"));
  Serial.println("  FB:   " + String(system_state.firebase_connected ? "Connected" : "Offline"));
  Serial.println("============================================");
  Serial.flush();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Process serial test commands from USB
  processSerialCommands();

  // Check WiFi connection and maintain it
  checkWiFiConnection();

  // Update Firebase connection state
  if (system_state.wifi_connected && firebaseInitialized) {
    system_state.firebase_connected = Firebase.ready();
  } else {
    system_state.firebase_connected = false;
  }

  // Process QR code scanner input (non-blocking)
  processQRScanner();

  // Process GSM responses (non-blocking)
  processGSM();

  // Monitor door sensors (reed switches)
  checkDoorSensors();

  // Continuous reed switch monitoring output
  if (reed1_monitor || reed2_monitor) {
    if (millis() - lastReedPrint >= 500) {
      lastReedPrint = millis();
      if (reed1_monitor) {
        bool s = digitalRead(DOOR_SENSOR_1_PIN) == HIGH;
        Serial.println("[REED-1] " + String(s ? "OPEN" : "CLOSED"));
        Serial.flush();
      }
      if (reed2_monitor) {
        bool s = digitalRead(DOOR_SENSOR_2_PIN) == HIGH;
        Serial.println("[REED-2] " + String(s ? "OPEN" : "CLOSED"));
        Serial.flush();
      }
    }
  }

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

  delay(50);
}

// ============================================================================
// SERIAL COMMAND TESTING SYSTEM
// ============================================================================
/*
 * Non-blocking: readStringUntil('\n') is called ONLY when Serial.available().
 * With Serial.setTimeout(50), it returns within 50ms max — never blocks loop.
 * Works with ANY Serial Monitor line-ending setting (NL, CR, or Both).
 *
 * Commands:
 *   relay-1:on / relay-1:off      Control lock relay 1
 *   relay-2:on / relay-2:off      Control lock relay 2
 *   buzzer:on  / buzzer:off       Buzzer control
 *   lcd:test                      LCD test display
 *   reed-1:read                   Single reed switch 1 read
 *   reed-1:mon:on / reed-1:mon:off  Continuous reed-1 monitor
 *   reed-2:read
 *   reed-2:mon:on / reed-2:mon:off
 *   qr:mon:on / qr:mon:off        Print raw QR scanner data
 *   gsm:<AT_CMD>                  Send AT command (e.g. gsm:AT+CSQ)
 *   gsm:mon:on / gsm:mon:off      Live GSM response monitor
 *   status                        System health check
 *   help                          Show this help
 */
void processSerialCommands() {
  // ------------------------------------------------------------------
  // Pattern identical to the sample:
  //   if (Serial.available()) { readStringUntil('\n'); process; }
  // Serial.setTimeout(50) keeps the 50ms worst-case latency.
  // .trim() removes the trailing '\r' so any line-ending setting works.
  // ------------------------------------------------------------------
  if (Serial.available()) {
    String rawLine = Serial.readStringUntil('\n');
    rawLine.trim();
    if (rawLine.length() == 0) return;

    String cmd = rawLine;
    cmd.toLowerCase();  // Case-insensitive matching

    Serial.print("[Serial] Received: "); Serial.println(rawLine); Serial.flush();

    // --- Relay 1 ---
    if (cmd == "relay-1:on") {
      openLock(1);
      Serial.println("[RELAY-1] ON"); Serial.flush();
    }
    else if (cmd == "relay-1:off") {
      closeLock(1);
      Serial.println("[RELAY-1] OFF"); Serial.flush();
    }
    // --- Relay 2 ---
    else if (cmd == "relay-2:on") {
      openLock(2);
      Serial.println("[RELAY-2] ON"); Serial.flush();
    }
    else if (cmd == "relay-2:off") {
      closeLock(2);
      Serial.println("[RELAY-2] OFF"); Serial.flush();
    }
    // --- Buzzer ---
    else if (cmd == "buzzer:on") {
      ledcWriteTone(BUZZER_PIN, 1000);
      Serial.println("[BUZZER] ON (1kHz continuous)"); Serial.flush();
    }
    else if (cmd == "buzzer:off") {
      ledcWriteTone(BUZZER_PIN, 0);
      Serial.println("[BUZZER] OFF"); Serial.flush();
    }
    // --- LCD ---
    else if (cmd == "lcd:test") {
      displayLCD("LCD TEST", "Line 2 OK", "Line 3 OK", "Line 4 OK");
      Serial.println("[LCD] Test pattern displayed"); Serial.flush();
    }
    // --- Reed Switch 1 ---
    else if (cmd == "reed-1:read") {
      bool s = digitalRead(DOOR_SENSOR_1_PIN) == HIGH;
      Serial.println("[REED-1] " + String(s ? "OPEN" : "CLOSED")); Serial.flush();
    }
    else if (cmd == "reed-1:mon:on") {
      reed1_monitor = true;
      Serial.println("[REED-1] Monitor ON (every 500ms)"); Serial.flush();
    }
    else if (cmd == "reed-1:mon:off") {
      reed1_monitor = false;
      Serial.println("[REED-1] Monitor OFF"); Serial.flush();
    }
    // --- Reed Switch 2 ---
    else if (cmd == "reed-2:read") {
      bool s = digitalRead(DOOR_SENSOR_2_PIN) == HIGH;
      Serial.println("[REED-2] " + String(s ? "OPEN" : "CLOSED")); Serial.flush();
    }
    else if (cmd == "reed-2:mon:on") {
      reed2_monitor = true;
      Serial.println("[REED-2] Monitor ON (every 500ms)"); Serial.flush();
    }
    else if (cmd == "reed-2:mon:off") {
      reed2_monitor = false;
      Serial.println("[REED-2] Monitor OFF"); Serial.flush();
    }
    // --- QR Scanner Monitor ---
    else if (cmd == "qr:mon:on") {
      qr_monitor = true;
      Serial.println("[QR] Monitor ON"); Serial.flush();
    }
    else if (cmd == "qr:mon:off") {
      qr_monitor = false;
      Serial.println("[QR] Monitor OFF"); Serial.flush();
    }
    // --- GSM Monitor ---
    else if (cmd == "gsm:mon:on") {
      gsm_monitor = true;
      Serial.println("[GSM] Monitor ON - live responses forwarded"); Serial.flush();
    }
    else if (cmd == "gsm:mon:off") {
      gsm_monitor = false;
      Serial.println("[GSM] Monitor OFF"); Serial.flush();
    }
    // --- GSM AT command: forward to SIM800L (must be AFTER gsm:mon: checks) ---
    else if (cmd.startsWith("gsm:")) {
      String atCmd = rawLine.substring(4);  // preserve casing for AT commands
      atCmd.trim();
      Serial.print("[GSM] Sending: "); Serial.println(atCmd); Serial.flush();
      sim800l.println(atCmd);              // send to UART2 (GPIO16 RX / GPIO17 TX)
      gsmRespTimeout = millis() + 2000;    // show responses for next 2s
    }
    // --- Status ---
    else if (cmd == "status") {
      checkSystemHealth();
    }
    // --- Help ---
    else if (cmd == "help") {
      printHelp();
    }
    else {
      Serial.println("[ERR] Unknown: '" + cmd + "' | Type help");
      Serial.flush();
    }

  }  // end if (Serial.available())
}

void printHelp() {
  Serial.println();
  Serial.println("======= PARCEL LOCKER COMMANDS =======");
  Serial.println("relay-1:on          Open relay 1 (lock 1)");
  Serial.println("relay-1:off         Close relay 1 (lock 1)");
  Serial.println("relay-2:on          Open relay 2 (lock 2)");
  Serial.println("relay-2:off         Close relay 2 (lock 2)");
  Serial.println("buzzer:on           Buzzer continuous 1kHz ON");
  Serial.println("buzzer:off          Buzzer OFF");
  Serial.println("lcd:test            Display test text on LCD");
  Serial.println("reed-1:read         Single read of reed switch 1");
  Serial.println("reed-1:mon:on       Start continuous reed-1 monitor");
  Serial.println("reed-1:mon:off      Stop continuous reed-1 monitor");
  Serial.println("reed-2:read         Single read of reed switch 2");
  Serial.println("reed-2:mon:on       Start continuous reed-2 monitor");
  Serial.println("reed-2:mon:off      Stop continuous reed-2 monitor");
  Serial.println("qr:mon:on           Print raw QR scanner data");
  Serial.println("qr:mon:off          Stop printing QR data");
  Serial.println("gsm:<AT CMD>        Send AT command to SIM800L");
  Serial.println("                    e.g.  gsm:AT   gsm:AT+CSQ");
  Serial.println("gsm:mon:on          Forward all live GSM responses");
  Serial.println("gsm:mon:off         Stop live GSM forwarding");
  Serial.println("status              System health check");
  Serial.println("help                Show this help");
  Serial.println("======================================");
  Serial.println("TIP: Line ending: Newline, CR, or Both NL & CR");
  Serial.println();
  Serial.flush();
}

// ============================================================================
// LOCK CONTROL (Direct Relay GPIO)
// ============================================================================
/*
 * Relay Module Logic (Active-LOW):
 * - GPIO LOW  = Relay ACTIVE   = Solenoid energized (lock opens)
 * - GPIO HIGH = Relay INACTIVE = Solenoid de-energized (lock closed)
 */
void openLock(int lockNum) {
  int pin = (lockNum == 1) ? RELAY_1_PIN : RELAY_2_PIN;
  digitalWrite(pin, LOW);  // Relay active → lock opens

  if (lockNum == 1) system_state.lock1_open = true;
  else system_state.lock2_open = true;

  debugPrint("Lock " + String(lockNum) + " OPENED");
  playBuzzer("click");
}

void closeLock(int lockNum) {
  int pin = (lockNum == 1) ? RELAY_1_PIN : RELAY_2_PIN;
  digitalWrite(pin, HIGH);  // Relay inactive → lock closed

  if (lockNum == 1) system_state.lock1_open = false;
  else system_state.lock2_open = false;

  debugPrint("Lock " + String(lockNum) + " CLOSED");
}

// ============================================================================
// BUZZER CONTROL (ESP32 LEDC PWM)
// ============================================================================
void playBuzzer(String tone_type) {
  if (tone_type == "startup") {
    // Startup: 2 short beeps
    for (int i = 0; i < 2; i++) {
      ledcWriteTone(BUZZER_PIN, 1000);
      delay(100);
      ledcWriteTone(BUZZER_PIN, 0);
      delay(50);
    }
  }
  else if (tone_type == "success") {
    // Success: ascending chirp
    for (int freq = 800; freq < 2000; freq += 50) {
      ledcWriteTone(BUZZER_PIN, freq);
      delay(20);
    }
  }
  else if (tone_type == "alert") {
    // Alert: rapid beeping
    for (int i = 0; i < 5; i++) {
      ledcWriteTone(BUZZER_PIN, 2000);
      delay(50);
      ledcWriteTone(BUZZER_PIN, 0);
      delay(50);
    }
  }
  else if (tone_type == "click") {
    // Click: single short beep
    ledcWriteTone(BUZZER_PIN, 1500);
    delay(50);
  }

  ledcWriteTone(BUZZER_PIN, 0);  // Ensure buzzer is off
}

// ============================================================================
// DOOR SENSOR MONITORING (Reed Switches - Direct GPIO)
// ============================================================================
void checkDoorSensors() {
  // Read sensors (LOW = closed/magnet near, HIGH = open/no magnet)
  bool new_door1 = digitalRead(DOOR_SENSOR_1_PIN) == HIGH;
  bool new_door2 = digitalRead(DOOR_SENSOR_2_PIN) == HIGH;

  // Detect state changes - door 1
  if (new_door1 != system_state.door1_open) {
    system_state.door1_open = new_door1;
    if (system_state.door1_open) {
      debugPrint("Parcel door OPENED");
    } else {
      debugPrint("Parcel door CLOSED");
      Serial.println("EVENT:DOOR1_CLOSED");
      handleDoorClosed(1);
    }
  }

  // Detect state changes - door 2
  if (new_door2 != system_state.door2_open) {
    system_state.door2_open = new_door2;
    if (system_state.door2_open) {
      debugPrint("Payment box door OPENED");
    } else {
      debugPrint("Payment box door CLOSED");
      Serial.println("EVENT:DOOR2_CLOSED");
      handleDoorClosed(2);
    }
  }
}

void handleDoorClosed(int doorNum) {
  if (doorNum == 1) {
    system_state.lock1_open = false;

    debugPrint("Parcel door closed - marking as delivered");
    displayLCD("Parcel Locked", "Delivery complete", "", "");

    // Log delivery event
    logParcelHistory(system_state.current_parcel_id, "PARCEL_DELIVERED");

    // Update parcel status in Firebase
    if (system_state.firebase_connected && system_state.current_parcel_id.length() > 0) {
      String parcelPath = String(ParcelBoxFirebaseConfig::getParcelsDatabasePath()) +
                          "/" + system_state.current_parcel_id + "/status";
      Firebase.RTDB.setString(&fbdo, parcelPath.c_str(), "delivered");
    }

    delay(2000);
    displayLCD("READY", "Scan parcel QR",
               "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"),
               "FB: " + String(system_state.firebase_connected ? "OK" : "---"));

    // Reset for next parcel
    system_state.current_parcel_id = "";
    system_state.current_qr_code = "";
  }
  else if (doorNum == 2) {
    system_state.lock2_open = false;
    debugPrint("Payment box closed");
  }
}

// ============================================================================
// SIM800L GSM FUNCTIONS (HardwareSerial UART2)
// ============================================================================
void initSIM800L() {
  Serial.print("[SIM800L] Resetting"); Serial.flush();
  digitalWrite(SIM800L_RST_PIN, LOW);
  delay(100);
  digitalWrite(SIM800L_RST_PIN, HIGH);

  // Boot wait — print a dot every 500ms so Serial Monitor shows progress
  for (int i = 0; i < 8; i++) {
    delay(500);
    Serial.print("."); Serial.flush();
  }
  Serial.println(" boot done"); Serial.flush();

  // Send AT to verify
  Serial.print("[SIM800L] Sending AT ... "); Serial.flush();
  sim800l.println("AT");
  delay(500);

  // Print any response
  if (sim800l.available()) {
    String resp = sim800l.readStringUntil('\n');
    resp.trim();
    Serial.println("response: " + resp); Serial.flush();
  } else {
    Serial.println("no response (check wiring/power)"); Serial.flush();
  }

  // Drain any remaining bytes
  while (sim800l.available()) sim800l.read();
}

void resetSIM800L() {
  Serial.print("[SIM800L] Hardware reset"); Serial.flush();
  digitalWrite(SIM800L_RST_PIN, LOW);
  delay(100);
  digitalWrite(SIM800L_RST_PIN, HIGH);
  delay(1000);
  Serial.println(" done"); Serial.flush();
}

void sendSMS(String phone, String message) {
  // Cooldown to prevent SMS spam
  unsigned long current_time = millis();
  if (current_time - system_state.last_sms_time < SMS_COOLDOWN) {
    debugPrint("SMS cooldown active, skipping send");
    return;
  }

  debugPrint("Sending SMS to: " + phone);
  debugPrint("Message: " + message);

  // SIM800L AT Command sequence
  sim800l.println("AT+CMGF=1");  // Set text mode
  delay(100);

  sim800l.print("AT+CMGS=\"");
  sim800l.print(phone);
  sim800l.println("\"");
  delay(100);

  sim800l.print(message);
  sim800l.write(26);  // Ctrl+Z to send
  delay(1000);

  system_state.last_sms_time = current_time;

  debugPrint("SMS sent!");
  playBuzzer("click");
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

  if (line1.length() > 0) {
    line1 = line1.substring(0, LCD_COLS);
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }

  if (line2.length() > 0) {
    line2 = line2.substring(0, LCD_COLS);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }

  if (line3.length() > 0) {
    line3 = line3.substring(0, LCD_COLS);
    lcd.setCursor(0, 2);
    lcd.print(line3);
  }

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

  Serial.println("[FB] Configuring Firebase..."); Serial.flush();

  // --- Same pattern as the reference (RiceDryer) ---
  // Uses Database Secret (legacy_token) instead of email/password anonymous auth.
  // Get your Database Secret:
  //   Firebase Console -> Project Settings -> Service accounts -> Database secrets
  config.host          = ParcelBoxFirebaseConfig::getFirebaseHost();
  config.database_url  = ParcelBoxFirebaseConfig::getDatabaseURL();
  config.signer.tokens.legacy_token = ParcelBoxFirebaseConfig::getFirebaseAuth();
  config.timeout.serverResponse = 10 * 1000;  // 10-second server response timeout

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("[FB] Firebase.begin() called — waiting for ready..."); Serial.flush();

  // Wait up to 10 seconds for Firebase to become ready
  unsigned long t = millis();
  while (!Firebase.ready() && millis() - t < 10000) {
    Serial.print("."); Serial.flush();
    delay(500);
  }
  Serial.println(); Serial.flush();

  if (Firebase.ready()) {
    Serial.println("[FB] Firebase CONNECTED!"); Serial.flush();
    system_state.firebase_connected = true;
    firebaseInitialized = true;
    registerDeviceInFirebase();
  } else {
    Serial.println("[FB] Firebase FAILED - check Database Secret and RTDB URL."); Serial.flush();
    Serial.println("[FB] Hint: get DB secret from Firebase Console -> Project Settings -> Service accounts"); Serial.flush();
    system_state.firebase_connected = false;
    // Still mark initialized so we don't retry on every loop; loop will retry via Firebase.ready()
    firebaseInitialized = true;
  }
}

void registerDeviceInFirebase() {
  String devicePath = ParcelBoxFirebaseConfig::getDeviceStatusPath();

  fbdo.setBSSLBufferSize(2048, 1024);
  fbdo.setResponseSize(2048);

  // Create device status object
  FirebaseJson json;
  json.set("device_id", system_state.device_id);
  json.set("model", "ParcelBox_ESP32");
  json.set("version", "2.0.0");
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
// UPDATE FIREBASE STATUS
// ============================================================================
void updateFirebaseStatus() {
  if (!system_state.firebase_connected) {
    return;
  }

  String devicePath = ParcelBoxFirebaseConfig::getDeviceStatusPath();

  // Update device heartbeat
  Firebase.RTDB.setInt(&fbdo, (devicePath + "/last_heartbeat").c_str(), millis());

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
// QR CODE SCANNER PROCESSING (UART1: GPIO33 RX / GPIO26 TX / GPIO25 RST)
// ============================================================================
/*
 * Non-blocking: readStringUntil('\n') called ONLY when qrScanner.available().
 * With qrScanner.setTimeout(100), returns within 100ms even without newline.
 * Each call reads ONE complete QR code line per loop iteration.
 */
void processQRScanner() {
  // Same pattern as sample: check available() first, then readStringUntil.
  // UART1 (GPIO33 RX / GPIO26 TX / GPIO25 RST) — does NOT interfere with GSM or Serial.
  if (qrScanner.available()) {
    String qr_code = qrScanner.readStringUntil('\n');
    qr_code.trim();

    if (qr_code.length() > 0) {
      Serial.print("[QR] Received: "); Serial.println(qr_code); Serial.flush();
      handleParcelScanned(qr_code);
    }
  }
}

// ============================================================================
// GSM RESPONSE PROCESSOR (UART2: GPIO16 RX / GPIO17 TX)
// ============================================================================
/*
 * Non-blocking: readStringUntil('\n') called ONLY when sim800l.available().
 * Prints GSM responses to Serial if:
 *   - gsm_monitor is ON (live monitoring), OR
 *   - gsmRespTimeout is active (2s window after sending an AT command).
 */
void processGSM() {
  // Same pattern as sample: check available() first, then readStringUntil.
  // UART2 (GPIO16 RX / GPIO17 TX) — does NOT interfere with QR or Serial.
  // Responses are printed when gsm_monitor is ON or within 2s of an AT command.
  if (sim800l.available()) {
    String line = sim800l.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      if (gsm_monitor || millis() < gsmRespTimeout) {
        Serial.print("[GSM] Received: "); Serial.println(line); Serial.flush();
      }
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

  bool is_valid = false;

  // Check Firebase for parcel information
  if (system_state.firebase_connected) {
    String parcelPath = String(ParcelBoxFirebaseConfig::getParcelsDatabasePath()) + "/" + qr_code;

    if (Firebase.RTDB.getJSON(&fbdo, parcelPath.c_str())) {
      if (fbdo.dataType() == "json") {
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
    openLock(1);
    delay(LOCK_OPERATION_DELAY);

    // Open payment box (Lock #2)
    openLock(2);
    delay(LOCK_OPERATION_DELAY);

    // Success feedback
    playBuzzer("success");
    displayLCD("DOORS OPEN", "Place parcel in box", "Complete payment", "Door closes auto");

    // Update Firebase
    updateFirebaseStatus();

  } else {
    debugPrint("QR Validation: FAILED");
    system_state.valid_scan = false;

    // Access denied
    playBuzzer("alert");
    displayLCD("Access Denied", "Invalid QR Code", "Try again", "");

    logParcelHistory(qr_code, "VALIDATION_FAILED");
    delay(3000);
    displayLCD("READY", "Scan parcel QR",
               "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"),
               "FB: " + String(system_state.firebase_connected ? "OK" : "---"));
  }
}

// ============================================================================
// SYSTEM HEALTH CHECK
// ============================================================================
void checkSystemHealth() {
  Serial.println();
  Serial.println("=== System Health ===");
  Serial.println("Uptime:    " + String(millis() / 1000) + "s");
  Serial.println("WiFi:      " + String(system_state.wifi_connected ? "Connected" : "Disconnected"));
  Serial.println("Firebase:  " + String(system_state.firebase_connected ? "Connected" : "Disconnected"));
  Serial.println("Parcel ID: " + (system_state.current_parcel_id.length() > 0 ? system_state.current_parcel_id : "None"));
  Serial.println("Lock 1:    " + String(system_state.lock1_open ? "OPEN" : "CLOSED"));
  Serial.println("Lock 2:    " + String(system_state.lock2_open ? "OPEN" : "CLOSED"));
  Serial.println("Door 1:    " + String(system_state.door1_open ? "OPEN" : "CLOSED"));
  Serial.println("Door 2:    " + String(system_state.door2_open ? "OPEN" : "CLOSED"));
  Serial.println("Reed-1 mon:" + String(reed1_monitor ? "ON" : "OFF"));
  Serial.println("Reed-2 mon:" + String(reed2_monitor ? "ON" : "OFF"));
  Serial.println("QR mon:    " + String(qr_monitor ? "ON" : "OFF"));
  Serial.println("GSM mon:   " + String(gsm_monitor ? "ON" : "OFF"));
  Serial.println("GSM pins:  RX=16 TX=17 | QR pins: RX=33 TX=26 RST=25");
  Serial.println("=====================");
  Serial.println();
  Serial.flush();
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
  Serial.flush();
}

// ============================================================================
// ADDITIONAL CONTROL FUNCTIONS
// ============================================================================
void closeLocksAfterDelivery() {
  debugPrint("Closing locks after delivery...");
  closeLock(1);
  delay(LOCK_OPERATION_DELAY);
  closeLock(2);
}

void emergencyLockdown() {
  debugPrint("EMERGENCY LOCKDOWN ACTIVATED!");
  displayLCD("LOCKDOWN", "System Secured", "Contact Admin", "");

  closeLock(1);
  delay(LOCK_OPERATION_DELAY);
  closeLock(2);
  playBuzzer("alert");

  logParcelHistory("SYSTEM", "EMERGENCY_LOCKDOWN");
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
