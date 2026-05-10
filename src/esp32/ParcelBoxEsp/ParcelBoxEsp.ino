#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <Preferences.h>

// Firebase library
#include <Firebase_ESP_Client.h>
// NOTE: TokenHelper.h and RTDBHelper.h are NOT included here.
// Their functions (printResult, tokenStatusCallback, getTokenType, etc.)
// are NOT called anywhere in the main code — including them causes
// multiple-definition linker errors since FirebaseManager.cpp also
// pulls them in. Only include if you actually call those helpers.

// Custom configuration modules
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "WiFiManagerCustom.h"
#include "ESPNOW_CONFIG.h"

// ESP-NOW library
#include <esp_now.h>
#include <esp_wifi.h>

// ============================================================================
// HARDWARE SERIAL PORTS
// ============================================================================
HardwareSerial sim800l(2);    // UART2 for SIM800L (GPIO16 RX / GPIO17 TX)
HardwareSerial qrScanner(1);  // UART1 for QR Scanner (GPIO33 RX / GPIO26 TX)

// ============================================================================
// SYSTEM STATE & SETTINGS
// ============================================================================
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
  String current_receiver_phone = "";
  String current_receiver_name = "";

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

  // SMS trigger counters & flags
  int invalid_scan_count = 0;
  bool door_breach_alerted = false;     // already sent breach SMS for this breach event
  bool valid_delivery_sms_sent = false; // already sent delivery-success SMS for this parcel
} system_state;

// ============================================================================
// FIREBASE CONNECTION STATE
// ============================================================================
bool firebaseInitialized = false;
bool firebaseStreamReady = false;  // Tracks that streams were started

// ============================================================================
// ESP-NOW — SINGLE MANAGER
// ============================================================================
// Single callback-based ESP-NOW. No EspNowManager conflicts.
volatile bool espNowQrAvailable = false;
ESPNOW_QRPacket_t espNowPacket;
esp_now_peer_info_t camPeerInfo;

// Duplicate scan protection
static String lastProcessedEspNowQR = "";
static unsigned long lastEspNowProcessTime = 0;

// Firebase singleton objects (global for callback access)
FirebaseData fbdo;
FirebaseData commandStream;
FirebaseAuth auth;
FirebaseConfig fbConfig;

// Stream state
bool commandStreamActive = false;

// ============================================================================
// SERIAL MONITOR FLAGS
// ============================================================================
bool reed1_monitor = false;
bool reed2_monitor = false;
bool qr_monitor = false;
bool gsm_monitor = false;
unsigned long gsmRespTimeout = 0;
unsigned long lastReedPrint = 0;

// ============================================================================
// TIMING INTERVALS
// ============================================================================
const unsigned long FIREBASE_UPDATE_INTERVAL = 5000;       // 5 seconds
const unsigned long HEALTH_CHECK_INTERVAL = 30000;          // 30 seconds
const unsigned long RECONNECT_CHECK_INTERVAL = 10000;       // 10 seconds
const unsigned long QR_SCAN_TIMEOUT = 30000;                // 30 seconds

unsigned long lastFirebaseUpdate = 0;
unsigned long lastHealthCheck = 0;
unsigned long lastReconnectCheck = 0;

// ============================================================================
// STATE TRACKING FOR DIRTY-CHECK (prevents redundant Firebase writes)
// ============================================================================
bool prev_lock1 = false;
bool prev_lock2 = false;
bool prev_door1 = false;
bool prev_door2 = false;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================
void setup();
void loop();

void setupWiFi();
void initializeNTP();
void checkWiFiConnection();
void reconnectWiFi();

void setupI2C_LCD();
void displayLCD(String line1, String line2 = "", String line3 = "", String line4 = "");

void openLock(int lockNum);
void closeLock(int lockNum);
void playBuzzer(String tone_type);
void checkDoorSensors();
void handleDoorClosed(int doorNum);
void initSIM800L();
void resetSIM800L();
void sendSMS(String phone, String message);

void processGSM();
void handleParcelScanned(String qr_code);
void validateAndOpenLocks(String qr_code);
void closeLocksAfterDelivery();
void emergencyLockdown();
void resetSystem();

void processSerialCommands();
void printHelp();

// ESP-NOW — SINGLE PATH
void setupEspNow();
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onEspNowSent(const wifi_tx_info_t *wifi_tx_info, esp_now_send_status_t status);
#else
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status);
#endif
void onEspNowReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void processEspNowQR();
void syncEspNowChannel();

// Firebase — SINGLE PATH
void initializeFirebase();
void registerDeviceInFirebase();
void updateFirebaseStatus();
void logParcelHistory(String parcel_id, String event);

// Firebase stream callbacks (static)
void commandStreamCallback(MultiPathStream stream);
void commandStreamTimeoutCallback(bool timeout);
void initCommandStream();
void handleFirebaseStream();

// Lock command callbacks
void onLockCommandFromFirebase(int lockNum, bool open);
void onEmergencyFromFirebase();

void generateDeviceId();
void checkSystemHealth();
void debugPrint(String msg);

// ============================================================================
// SMS TRIGGER FUNCTIONS
// ============================================================================
void smsSendValidDelivery();
void smsSendInvalidAttempt();
void smsSendDoorBreach();

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
  Serial.begin(BAUD_SERIAL);
  Serial.setTimeout(50);
  delay(500);

  Serial.println(F("================================================\n"
                   "Smart Parcel Locker - ESP32 Startup\n"
                   "================================================"));

  // ── Step 1: GPIO ──────────────────────────────────────────────────────────
  Serial.println(F("[SETUP 1/7] Initializing GPIO pins..."));
  pinMode(DOOR_SENSOR_1_PIN, INPUT_PULLUP);
  pinMode(DOOR_SENSOR_2_PIN, INPUT_PULLUP);
  pinMode(SIM800L_RST_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(SIM800L_RST_PIN, HIGH);
  ledcAttach(BUZZER_PIN, 1000, BUZZER_RESOLUTION);
  Serial.println(F("[SETUP 1/7] GPIO OK"));

  // ── Step 2: UARTs ─────────────────────────────────────────────────────────
  Serial.println(F("[SETUP 2/7] Initializing UARTs..."));
  sim800l.begin(BAUD_SIM800L, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  sim800l.setTimeout(50);
  qrScanner.setTimeout(100);
  Serial.println(F("[SETUP 2/7] UARTs OK"));

  // ── Step 3: I2C + LCD ────────────────────────────────────────────────────
  Serial.println(F("[SETUP 3/7] Initializing I2C + LCD..."));
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  setupI2C_LCD();
  displayLCD("PARCEL LOCKER", "Initializing...", "v2.0 (ESP32)", "");
  delay(1000);
  Serial.println(F("[SETUP 3/7] LCD OK"));

  // ── Step 4: Device ID ────────────────────────────────────────────────────
  Serial.println(F("[SETUP 4/7] Generating Device ID..."));
  generateDeviceId();
  Serial.println("[SETUP 4/7] Device ID: " + system_state.device_id);

  // ── Step 5: SIM800L ──────────────────────────────────────────────────────
  Serial.println(F("[SETUP 5/7] Initializing SIM800L..."));
  initSIM800L();
  Serial.println(F("[SETUP 5/7] SIM800L OK"));
  playBuzzer("startup");

  // ── Step 6: WiFi ─────────────────────────────────────────────────────────
  Serial.println(F("[SETUP 6/7] Starting WiFi..."));
  displayLCD("Setting up WiFi", "Join: ParcelBox", "_Setup or wait...", "pw: password123");
  setupWiFi();
  Serial.println("[SETUP 6/7] WiFi result: " + String(system_state.wifi_connected ? "CONNECTED" : "OFFLINE"));

  // ── Step 7: ESP-NOW + Firebase ──────────────────────────────────────────
  if (system_state.wifi_connected) {
    Serial.println(F("[SETUP 7/7] Initializing Communication System..."));
    displayLCD("Initializing", "Communications", "", "");

    // STEP 7a: ESP-NOW first (needs WiFi in STA mode)
    setupEspNow();

    // STEP 7b: Firebase second
    initializeFirebase();

    Serial.println("[SETUP 7/7] Communication result: " +
      String(firebaseInitialized ? "CONNECTED" : "FAILED"));
  } else {
    Serial.println(F("[SETUP 7/7] Communication SKIPPED (no WiFi)"));
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
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  processSerialCommands();
  checkWiFiConnection();

  // ESP-NOW QR from ESP32-CAM (highest priority)
  processEspNowQR();

  // Process GSM responses
  processGSM();

  // Monitor door sensors
  checkDoorSensors();

  // Continuous reed switch monitoring output (only when enabled)
  if (reed1_monitor || reed2_monitor) {
    if (millis() - lastReedPrint >= 500) {
      lastReedPrint = millis();
      if (reed1_monitor) {
        Serial.println("[REED-1] " + String(digitalRead(DOOR_SENSOR_1_PIN) == HIGH ? "OPEN" : "CLOSED"));
      }
      if (reed2_monitor) {
        Serial.println("[REED-2] " + String(digitalRead(DOOR_SENSOR_2_PIN) == HIGH ? "OPEN" : "CLOSED"));
      }
    }
  }

  // ── Firebase: handle streaming command channel ──────────────────────────
  if (system_state.firebase_connected) {
    handleFirebaseStream();
  }

  // ── Firebase: update lock status (THROTTLED + DIRTY-CHECKED) ────────────
  if (system_state.firebase_connected && firebaseInitialized) {
    system_state.firebase_connected = Firebase.ready();
    if (system_state.firebase_connected) {
      bool lock1 = system_state.lock1_open;
      bool lock2 = system_state.lock2_open;
      bool door1 = system_state.door1_open;
      bool door2 = system_state.door2_open;

      // Only write to Firebase when state actually changes
      if (lock1 != prev_lock1 || lock2 != prev_lock2 ||
          door1 != prev_door1 || door2 != prev_door2) {
        updateFirebaseStatus();
        prev_lock1 = lock1;
        prev_lock2 = lock2;
        prev_door1 = door1;
        prev_door2 = door2;
      }

      // Periodic heartbeat (every 30s)
      if (millis() - lastHealthCheck > HEALTH_CHECK_INTERVAL) {
        lastHealthCheck = millis();
        if (firebaseInitialized && Firebase.ready()) {
          String devPath = ParcelBoxFirebaseConfig::getDeviceStatusPath();
          Firebase.RTDB.setInt(&fbdo, (devPath + "/" + system_state.device_id + "/last_heartbeat").c_str(), millis());
        }
        checkSystemHealth();
      }
    }
  }

  delay(10);  // Reduced from 50ms — watchdog-friendly, better responsiveness
}

// ============================================================================
// SERIAL COMMAND PROCESSING
// ============================================================================
void processSerialCommands() {
  if (!Serial.available()) return;

  String rawLine = Serial.readStringUntil('\n');
  rawLine.trim();
  if (rawLine.length() == 0) return;

  String cmd = rawLine;
  cmd.toLowerCase();

  Serial.print("[Serial] Cmd: "); Serial.println(rawLine);

  if (cmd == "relay-1:on") { openLock(1); Serial.println(F("[RELAY-1] ON")); }
  else if (cmd == "relay-1:off") { closeLock(1); Serial.println(F("[RELAY-1] OFF")); }
  else if (cmd == "relay-2:on") { openLock(2); Serial.println(F("[RELAY-2] ON")); }
  else if (cmd == "relay-2:off") { closeLock(2); Serial.println(F("[RELAY-2] OFF")); }
  else if (cmd == "buzzer:on") { ledcWriteTone(BUZZER_PIN, 1000); Serial.println(F("[BUZZER] ON")); }
  else if (cmd == "buzzer:off") { ledcWriteTone(BUZZER_PIN, 0); Serial.println(F("[BUZZER] OFF")); }
  else if (cmd == "lcd:test") { displayLCD("LCD TEST", "Line 2 OK", "Line 3 OK", "Line 4 OK"); }
  else if (cmd == "reed-1:read") { Serial.println("[REED-1] " + String(digitalRead(DOOR_SENSOR_1_PIN) == HIGH ? "OPEN" : "CLOSED")); }
  else if (cmd == "reed-1:mon:on") { reed1_monitor = true; Serial.println(F("[REED-1] Monitor ON")); }
  else if (cmd == "reed-1:mon:off") { reed1_monitor = false; Serial.println(F("[REED-1] Monitor OFF")); }
  else if (cmd == "reed-2:read") { Serial.println("[REED-2] " + String(digitalRead(DOOR_SENSOR_2_PIN) == HIGH ? "OPEN" : "CLOSED")); }
  else if (cmd == "reed-2:mon:on") { reed2_monitor = true; Serial.println(F("[REED-2] Monitor ON")); }
  else if (cmd == "reed-2:mon:off") { reed2_monitor = false; Serial.println(F("[REED-2] Monitor OFF")); }
  else if (cmd == "qr:mon:on") { qr_monitor = true; Serial.println(F("[QR] Monitor ON")); }
  else if (cmd == "qr:mon:off") { qr_monitor = false; Serial.println(F("[QR] Monitor OFF")); }
  else if (cmd == "gsm:mon:on") { gsm_monitor = true; Serial.println(F("[GSM] Monitor ON")); }
  else if (cmd == "gsm:mon:off") { gsm_monitor = false; Serial.println(F("[GSM] Monitor OFF")); }
  else if (cmd.startsWith("gsm:")) {
    String atCmd = rawLine.substring(4);
    atCmd.trim();
    Serial.print("[GSM] Sending: "); Serial.println(atCmd);
    sim800l.println(atCmd);
    gsmRespTimeout = millis() + 2000;
  }
  else if (cmd == "status") { checkSystemHealth(); }
  else if (cmd == "help") { printHelp(); }
  else { Serial.println("[ERR] Unknown: '" + cmd + "' | Type help"); }
}

void printHelp() {
  Serial.println(F("======= PARCEL LOCKER COMMANDS ======="
                   "\nrelay-1:on/off         Control lock 1"
                   "\nrelay-2:on/off         Control lock 2"
                   "\nbuzzer:on/off          Buzzer control"
                   "\nlcd:test               LCD test display"
                   "\nreed-1:read            Read reed switch 1"
                   "\nreed-1:mon:on/off      Continuous reed-1 monitor"
                   "\nreed-2:read            Read reed switch 2"
                   "\nreed-2:mon:on/off      Continuous reed-2 monitor"
                   "\nqr:mon:on/off          Print raw QR scanner data"
                   "\ngsm:<AT CMD>           Send AT command to SIM800L"
                   "\ngsm:mon:on/off         Forward GSM responses"
                   "\nstatus                 System health check"
                   "\nhelp                   Show this help"
                   "\n======================================"));
}

// ============================================================================
// LOCK CONTROL
// ============================================================================
void openLock(int lockNum) {
  int pin = (lockNum == 1) ? RELAY_1_PIN : RELAY_2_PIN;
  digitalWrite(pin, LOW);
  if (lockNum == 1) system_state.lock1_open = true;
  else system_state.lock2_open = true;
  debugPrint("Lock " + String(lockNum) + " OPENED");
  playBuzzer("click");
}

void closeLock(int lockNum) {
  int pin = (lockNum == 1) ? RELAY_1_PIN : RELAY_2_PIN;
  digitalWrite(pin, HIGH);
  if (lockNum == 1) system_state.lock1_open = false;
  else system_state.lock2_open = false;
  debugPrint("Lock " + String(lockNum) + " CLOSED");
}

// ============================================================================
// BUZZER
// ============================================================================
void playBuzzer(String tone_type) {
  if (tone_type == "startup") {
    for (int i = 0; i < 2; i++) { ledcWriteTone(BUZZER_PIN, 1000); delay(100); ledcWriteTone(BUZZER_PIN, 0); delay(50); }
  } else if (tone_type == "success") {
    for (int freq = 800; freq < 2000; freq += 50) { ledcWriteTone(BUZZER_PIN, freq); delay(20); }
  } else if (tone_type == "alert") {
    for (int i = 0; i < 5; i++) { ledcWriteTone(BUZZER_PIN, 2000); delay(50); ledcWriteTone(BUZZER_PIN, 0); delay(50); }
  } else if (tone_type == "click") {
    ledcWriteTone(BUZZER_PIN, 1500); delay(50);
  }
  ledcWriteTone(BUZZER_PIN, 0);
}

// ============================================================================
// DOOR SENSORS
// ============================================================================
void checkDoorSensors() {
  bool nd1 = digitalRead(DOOR_SENSOR_1_PIN) == HIGH;
  bool nd2 = digitalRead(DOOR_SENSOR_2_PIN) == HIGH;

  // — Door 1 (parcel door) —
  if (nd1 != system_state.door1_open) {
    system_state.door1_open = nd1;
    if (system_state.door1_open) {
      debugPrint("Parcel door OPENED");
      // Door opened WITHOUT a valid scan — possible break-in
      if (!system_state.valid_scan) {
        smsSendDoorBreach();
      }
    } else {
      debugPrint("Parcel door CLOSED");
      handleDoorClosed(1);
    }
  }

  // — Door 2 (payment box door) —
  if (nd2 != system_state.door2_open) {
    system_state.door2_open = nd2;
    if (system_state.door2_open) {
      debugPrint("Payment box door OPENED");
      // Door opened WITHOUT a valid scan — possible break-in
      if (!system_state.valid_scan) {
        smsSendDoorBreach();
      }
    } else {
      debugPrint("Payment box door CLOSED");
      handleDoorClosed(2);
    }
  }

  // Reset breach alert flag when BOTH doors fully close & system resets to ready state
  if (!nd1 && !nd2 && system_state.current_parcel_id == "") {
    system_state.door_breach_alerted = false;
  }
}

void handleDoorClosed(int doorNum) {
  if (doorNum == 1) {
    system_state.lock1_open = false;
    debugPrint("Parcel door closed - marking as delivered");
    displayLCD("Parcel Locked", "Delivery complete", "", "");

    // Send SMS for valid delivery (locks opened via valid QR, door closed)
    if (system_state.valid_scan && !system_state.valid_delivery_sms_sent) {
      smsSendValidDelivery();
    }

    if (system_state.firebase_connected && system_state.current_parcel_id.length() > 0) {
      logParcelHistory(system_state.current_parcel_id, "PARCEL_DELIVERED");
    }

    delay(2000);
    displayLCD("READY", "Scan parcel QR",
               "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"),
               "FB: " + String(system_state.firebase_connected ? "OK" : "---"));
    system_state.current_parcel_id = "";
    system_state.current_qr_code = "";
    system_state.current_receiver_phone = "";
    system_state.current_receiver_name = "";
    system_state.valid_scan = false;
    system_state.valid_delivery_sms_sent = false;
  } else if (doorNum == 2) {
    system_state.lock2_open = false;
    debugPrint("Payment box closed");
  }
}

// ============================================================================
// SIM800L GSM
// ============================================================================
void initSIM800L() {
  Serial.print(F("[SIM800L] Resetting"));
  digitalWrite(SIM800L_RST_PIN, LOW);
  delay(100);
  digitalWrite(SIM800L_RST_PIN, HIGH);
  for (int i = 0; i < 8; i++) { delay(500); Serial.print("."); }
  Serial.println(F(" boot done"));

  Serial.print(F("[SIM800L] AT... "));
  sim800l.println("AT");
  delay(500);
  if (sim800l.available()) {
    Serial.println("response: " + sim800l.readStringUntil('\n'));
  } else {
    Serial.println(F("no response (check wiring/power)"));
  }
  while (sim800l.available()) sim800l.read();
}

void resetSIM800L() {
  Serial.print(F("[SIM800L] Hardware reset"));
  digitalWrite(SIM800L_RST_PIN, LOW); delay(100);
  digitalWrite(SIM800L_RST_PIN, HIGH); delay(1000);
  Serial.println(F(" done"));
}

void sendSMS(String phone, String message) {
  if (millis() - system_state.last_sms_time < SMS_COOLDOWN) {
    debugPrint("SMS cooldown active");
    return;
  }
  debugPrint("Sending SMS to: " + phone);
  sim800l.println("AT+CMGF=1"); delay(100);
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phone);
  sim800l.println("\""); delay(100);
  sim800l.print(message);
  sim800l.write(26);
  delay(1000);
  system_state.last_sms_time = millis();
  debugPrint("SMS sent!");
  playBuzzer("click");
}

// ============================================================================
// SMS TRIGGERS
// ============================================================================

/**
 * smsSendValidDelivery() — triggered when:
 *   - QR scan was VALID
 *   - Locks were opened
 *   - Door 1 (parcel door) closes
 * Sends to: receiver's contact number (from Firebase)
 */
void smsSendValidDelivery() {
  if (system_state.current_receiver_phone.length() == 0) {
    debugPrint("[SMS] No receiver phone — skipping delivery SMS");
    return;
  }
  String msg = "ParcelBox: Your parcel " + system_state.current_parcel_id +
               " has been delivered successfully. Please check locker " +
               system_state.current_qr_code + ". - ParcelBox System";
  sendSMS(system_state.current_receiver_phone, msg);
  system_state.valid_delivery_sms_sent = true;
  logParcelHistory(system_state.current_parcel_id, "SMS_DELIVERY_SENT");
}

/**
 * smsSendInvalidAttempt() — triggered when:
 *   - Invalid QR scanned 3 times in a row (consecutive)
 * Sends to: admin/monitoring number (device owner)
 */
void smsSendInvalidAttempt() {
  String msg = "[ALERT] ParcelBox " + system_state.device_id +
               ": 3 invalid QR attempts detected. Possible tampering. - ParcelBox System";
  // Send to a predefined monitoring/admin number
  // The admin number could be stored in Preferences; for now, use a placeholder.
  // Replace "+63XXXXXXXXXX" with your actual admin phone number:
  String adminPhone = "+639123456789";
  sendSMS(adminPhone, msg);
  logParcelHistory("SYSTEM", "SMS_INVALID_ATTEMPT_3X");
}

/**
 * smsSendDoorBreach() — triggered when:
 *   - ANY door opens WITHOUT a valid scan (no valid QR scan in progress)
 *   - Only sends once per breach event (door_breach_alerted flag)
 * Sends to: admin/monitoring number
 */
void smsSendDoorBreach() {
  if (system_state.door_breach_alerted) return;  // already alerted for this breach
  system_state.door_breach_alerted = true;

  String doorLabel = "";
  if (system_state.door1_open) doorLabel += "Parcel Door ";
  if (system_state.door2_open) doorLabel += "Payment Box ";
  if (doorLabel == "") doorLabel = "Unknown Door";

  String msg = "[BREACH ALERT] ParcelBox " + system_state.device_id +
               ": " + doorLabel + "opened without authorization! - ParcelBox System";
  String adminPhone = "+639123456789";
  sendSMS(adminPhone, msg);
  logParcelHistory("SYSTEM", "SMS_DOOR_BREACH");
}

// ============================================================================
// WIFI
// ============================================================================
void setupWiFi() {
  debugPrint("Initializing WiFi Manager...");
  if (!wifiManager.begin("ParcelBox_Setup", "password123")) {
    debugPrint("WiFi setup timeout");
    displayLCD("WiFi Timeout", "Will retry in", "offline mode", "");
    system_state.wifi_connected = false;
    return;
  }
  debugPrint("WiFi Connected!");
  debugPrint("IP Address: " + wifiManager.getLocalIP());
  displayLCD("WiFi Connected", "IP: " + wifiManager.getLocalIP(), "", "");
  system_state.wifi_connected = true;
  delay(2000);
  initializeNTP();
}

void initializeNTP() {
  debugPrint("Syncing time with NTP...");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 20) { delay(500); now = time(nullptr); attempts++; }
  struct tm timeinfo = *localtime(&now);
  debugPrint("Time: " + String(asctime(&timeinfo)));
}

void checkWiFiConnection() {
  unsigned long now = millis();
  if (now - lastReconnectCheck < RECONNECT_CHECK_INTERVAL) return;
  lastReconnectCheck = now;

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

    // Re-init ESP-NOW after WiFi reconnect (WiFi channel may have changed)
    setupEspNow();

    system_state.wifi_connected = true;
    if (!firebaseInitialized) {
      initializeFirebase();
    }
  }
}

void reconnectWiFi() {
  wifiManager.reset();
  delay(1000);
  setupWiFi();
}

// ============================================================================
// LCD
// ============================================================================
void setupI2C_LCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  debugPrint("LCD Initialized");
}

void displayLCD(String line1, String line2, String line3, String line4) {
  lcd.clear();
  if (line1.length() > 0) { lcd.setCursor(0, 0); lcd.print(line1.substring(0, LCD_COLS)); }
  if (line2.length() > 0) { lcd.setCursor(0, 1); lcd.print(line2.substring(0, LCD_COLS)); }
  if (line3.length() > 0) { lcd.setCursor(0, 2); lcd.print(line3.substring(0, LCD_COLS)); }
  if (line4.length() > 0) { lcd.setCursor(0, 3); lcd.print(line4.substring(0, LCD_COLS)); }
}

// ============================================================================
// ESP-NOW — SINGLE INIT, SINGLE CALLBACK
// ============================================================================
void setupEspNow() {
  esp_err_t err;

  // 1. Ensure WiFi is STA mode
  WiFi.mode(WIFI_STA);

  // 2. Deinit first if already initialized (safe for reinit after reconnect)
  esp_now_deinit();

  // 3. Init fresh
  err = esp_now_init();
  if (err != ESP_OK) {
    Serial.printf("[ESPNOW] Init failed: %d\n", err);
    return;
  }

  // 4. Register callbacks
  esp_now_register_send_cb(onEspNowSent);
  esp_now_register_recv_cb(onEspNowReceived);

  // 5. Add CAM as peer
  memset(&camPeerInfo, 0, sizeof(camPeerInfo));
  camPeerInfo.channel = 0;  // Will be locked via esp_wifi_set_channel()
  camPeerInfo.encrypt = false;
  camPeerInfo.ifidx = WIFI_IF_STA;
  camPeerInfo.peer_addr[0] = CAM_MAC_0;
  camPeerInfo.peer_addr[1] = CAM_MAC_1;
  camPeerInfo.peer_addr[2] = CAM_MAC_2;
  camPeerInfo.peer_addr[3] = CAM_MAC_3;
  camPeerInfo.peer_addr[4] = CAM_MAC_4;
  camPeerInfo.peer_addr[5] = CAM_MAC_5;

  err = esp_now_add_peer(&camPeerInfo);
  if (err != ESP_OK) {
    Serial.printf("[ESPNOW] Peer add failed: %d\n", err);
  }

  // 6. Channel sync — force ESP-NOW to WiFi channel
  syncEspNowChannel();

  Serial.printf("[ESPNOW] Ready. MAC: %s, Channel: %d\n",
    WiFi.macAddress().c_str(), WiFi.channel());
}

void syncEspNowChannel() {
  if (WiFi.status() == WL_CONNECTED) {
    int ch = WiFi.channel();
    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
    // Log only on change (no flooding)
    Serial.printf("[ESPNOW] Channel synced: %d\n", ch);
  }
}

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onEspNowSent(const wifi_tx_info_t *wifi_tx_info, esp_now_send_status_t status) {
#else
void onEspNowSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#endif
  // Only log failures to avoid flooding on ACKs
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println(F("[ESPNOW] Send FAILED"));
  }
}

void onEspNowReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // Verify packet size matches ESPNOW_QRPacket_t (42 bytes)
  if (len != sizeof(ESPNOW_QRPacket_t)) {
    // Silently discard mismatched packets — no serial spam
    return;
  }

  memcpy(&espNowPacket, data, sizeof(ESPNOW_QRPacket_t));

  // Sanitize QR data
  espNowPacket.qrData[31] = '\0';
  String qrClean = String(espNowPacket.qrData);
  qrClean.trim();
  qrClean.replace("\r", "");
  qrClean.replace("\n", "");
  strncpy(espNowPacket.qrData, qrClean.c_str(), 31);
  espNowPacket.qrData[31] = '\0';

  espNowQrAvailable = true;

  // Single clean print when QR arrives
  Serial.print(F("[QR RX] Received QR: "));
  Serial.println(espNowPacket.qrData);
}

void processEspNowQR() {
  if (!espNowQrAvailable) return;
  espNowQrAvailable = false;

  String qr_code = String(espNowPacket.qrData);

  // Duplicate protection
  if (qr_code == lastProcessedEspNowQR &&
      (millis() - lastEspNowProcessTime) < ESPNOW_DUPLICATE_COOLDOWN) {
    return;
  }

  lastProcessedEspNowQR = qr_code;
  lastEspNowProcessTime = millis();

  Serial.println("[ESPNOW] Processing QR: " + qr_code);
  displayLCD("QR via CAM", qr_code, "Validating...", "");
  handleParcelScanned(qr_code);
}

// ============================================================================
// FIREBASE — SINGLE PATH
// ============================================================================
void initializeFirebase() {
  if (firebaseInitialized) return;

  Serial.println(F("[FB] Initializing..."));

  // Configure
  fbConfig.host = ParcelBoxFirebaseConfig::getFirebaseHost();
  fbConfig.database_url = ParcelBoxFirebaseConfig::getDatabaseURL();
  fbConfig.signer.tokens.legacy_token = ParcelBoxFirebaseConfig::getFirebaseAuth();
  fbConfig.timeout.serverResponse = 10 * 1000;
  fbConfig.timeout.rtdbStreamReconnect = 1 * 1000;
  fbConfig.timeout.rtdbStreamError = 3 * 1000;

  // Set buffer sizes
  fbdo.setBSSLBufferSize(2048, 1024);
  fbdo.setResponseSize(2048);
  commandStream.setBSSLBufferSize(2048, 1024);
  commandStream.setResponseSize(2048);

  // Begin
  Firebase.begin(&fbConfig, &auth);
  Firebase.reconnectWiFi(true);

  Serial.print(F("[FB] Awaiting connection"));
  unsigned long start = millis();
  while (!Firebase.ready() && millis() - start < 10000) {
    Serial.print("."); delay(500);
  }
  Serial.println();

  if (Firebase.ready()) {
    Serial.println(F("[FB] ✅ Connected"));
    system_state.firebase_connected = true;
    firebaseInitialized = true;
    registerDeviceInFirebase();
    initCommandStream();
  } else {
    Serial.println(F("[FB] ❌ Connection FAILED"));
    firebaseInitialized = true;
  }
}

void registerDeviceInFirebase() {
  String path = String(ParcelBoxFirebaseConfig::getDeviceStatusPath()) + "/" + system_state.device_id;
  FirebaseJson json;
  json.set("device_id", system_state.device_id);
  json.set("model", "ParcelBox_ESP32");
  json.set("version", "2.0.0");
  json.set("wifi_connected", true);
  json.set("firebase_connected", true);
  json.set("last_heartbeat", millis());

  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    debugPrint("Device registered in Firebase");
  } else {
    debugPrint("Register failed: " + String(fbdo.errorReason()));
  }
}

void updateFirebaseStatus() {
  if (!system_state.firebase_connected || !Firebase.ready()) return;

  String path = String(ParcelBoxFirebaseConfig::getLocksStatusPath()) + "/" + system_state.device_id;
  FirebaseJson json;
  json.set("lock1", system_state.lock1_open ? "open" : "closed");
  json.set("lock2", system_state.lock2_open ? "open" : "closed");
  json.set("door1", system_state.door1_open ? "open" : "closed");
  json.set("door2", system_state.door2_open ? "closed" : "open");
  json.set("timestamp/.sv", "timestamp");

  if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    // Print only once per state change — no flooding
    debugPrint("FB: Lock status updated");
  } else {
    debugPrint("FB update failed: " + String(fbdo.errorReason()));
  }
}

void logParcelHistory(String parcel_id, String event) {
  if (!system_state.firebase_connected || !Firebase.ready()) return;

  String path = String(ParcelBoxFirebaseConfig::getHistoryPath()) + "/" + system_state.device_id;
  FirebaseJson entry;
  entry.set("parcel_id", parcel_id);
  entry.set("event", event);
  entry.set("timestamp/.sv", "timestamp");
  entry.set("device_id", system_state.device_id);

  if (Firebase.RTDB.pushJSON(&fbdo, path.c_str(), &entry)) {
    debugPrint("History: " + parcel_id + " -> " + event);
  } else {
    debugPrint("History failed: " + String(fbdo.errorReason()));
  }
}

// ============================================================================
// FIREBASE STREAM — COMMAND CHANNEL
// ============================================================================
void initCommandStream() {
  if (!firebaseInitialized || !Firebase.ready()) return;

  String cmdPath = String(ParcelBoxFirebaseConfig::getDeviceStatusPath()) +
    "/" + system_state.device_id + "/commands";

  Serial.println("[FB] Starting command stream: " + cmdPath);

  if (!Firebase.RTDB.beginMultiPathStream(&commandStream, cmdPath)) {
    Serial.printf("[FB] Stream init failed: %s\n", commandStream.errorReason().c_str());
    commandStreamActive = false;
  } else {
    Firebase.RTDB.setMultiPathStreamCallback(&commandStream, commandStreamCallback, commandStreamTimeoutCallback);
    commandStreamActive = true;
    Serial.println(F("[FB] ✅ Command stream active"));
  }
}

void handleFirebaseStream() {
  if (!commandStreamActive) return;

  if (!Firebase.RTDB.readStream(&commandStream)) {
    int httpCode = commandStream.httpCode();
    if (httpCode < 0 || httpCode == FIREBASE_ERROR_HTTP_CODE_OK) {
      // stream data just not available yet — normal
      return;
    }
    Serial.printf("[FB] Stream error: %s\n", commandStream.errorReason().c_str());
    commandStreamActive = false;
    // Reinit on next loop
    initCommandStream();
  }
}

void commandStreamCallback(MultiPathStream stream) {
  if (stream.get("/lock1")) {
    String cmd = stream.value;
    cmd.replace("\"", "");
    if (cmd == "open" || cmd == "close") {
      Serial.println("[FB] Lock1 cmd: " + cmd);
      if (cmd == "open") { onLockCommandFromFirebase(1, true); }
      else { onLockCommandFromFirebase(1, false); }
    }
  }
  if (stream.get("/lock2")) {
    String cmd = stream.value;
    cmd.replace("\"", "");
    if (cmd == "open" || cmd == "close") {
      Serial.println("[FB] Lock2 cmd: " + cmd);
      if (cmd == "open") { onLockCommandFromFirebase(2, true); }
      else { onLockCommandFromFirebase(2, false); }
    }
  }
  if (stream.get("/emergency_unlock")) {
    if (stream.value == "true") {
      Serial.println(F("[FB] Emergency unlock commanded"));
      onEmergencyFromFirebase();
    }
  }
}

void commandStreamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println(F("[FB] Stream timeout"));
  }
  if (!commandStream.httpConnected()) {
    Serial.printf("[FB] Stream disconnected: %d\n", commandStream.httpCode());
    commandStreamActive = false;
  }
}

// ============================================================================
// FIREBASE COMMAND CALLBACKS
// ============================================================================
void onLockCommandFromFirebase(int lockNum, bool open) {
  if (open) {
    openLock(lockNum);
    displayLCD("Remote: Lock " + String(lockNum), "Opening...", "", "");
    logParcelHistory("remote", "REMOTE_LOCK_" + String(lockNum) + "_OPEN");
  } else {
    closeLock(lockNum);
    displayLCD("Remote: Lock " + String(lockNum), "Closing...", "", "");
    logParcelHistory("remote", "REMOTE_LOCK_" + String(lockNum) + "_CLOSE");
  }
}

void onEmergencyFromFirebase() {
  emergencyLockdown();
}

// ============================================================================
// PARCEL QR WORKFLOW
// ============================================================================
void processGSM() {
  if (sim800l.available()) {
    String line = sim800l.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && (gsm_monitor || millis() < gsmRespTimeout)) {
      Serial.print("[GSM] "); Serial.println(line);
    }
  }
}

void handleParcelScanned(String qr_code) {
  system_state.current_qr_code = qr_code;
  system_state.last_scan_time = millis();

  displayLCD("QR SCANNED", qr_code, "Validating...", "");

  if (system_state.firebase_connected) {
    logParcelHistory(qr_code, "QR_SCANNED");
  }

  validateAndOpenLocks(qr_code);
}

void validateAndOpenLocks(String qr_code) {
  debugPrint("Validating QR: " + qr_code);

  bool is_valid = false;

  if (system_state.firebase_connected && Firebase.ready()) {
    String parcelPath = String(ParcelBoxFirebaseConfig::getParcelsDatabasePath()) + "/" + qr_code;
    if (Firebase.RTDB.getJSON(&fbdo, parcelPath.c_str())) {
      if (fbdo.dataType() == "json") {
        is_valid = true;
        system_state.current_parcel_id = qr_code;
        debugPrint("Parcel found in Firebase");

        // — Extract receiver contact info for SMS —
        FirebaseJson* jsonPtr = fbdo.to<FirebaseJson*>();
        if (jsonPtr) {
          FirebaseJsonData jsonData;
          if (jsonPtr->get(jsonData, "contact_number")) {
            system_state.current_receiver_phone = jsonData.stringValue;
          }
          if (jsonPtr->get(jsonData, "receiver_name")) {
            system_state.current_receiver_name = jsonData.stringValue;
          }
          debugPrint("Receiver phone: " + system_state.current_receiver_phone);
        }

        logParcelHistory(qr_code, "PARCEL_FOUND");

        // — Reset invalid scan counter on success —
        system_state.invalid_scan_count = 0;
      }
    }
  } else {
    if (qr_code.length() >= 5) {
      is_valid = true;
      system_state.current_parcel_id = qr_code;
      // Reset invalid scan counter on offline success
      system_state.invalid_scan_count = 0;
    }
  }

  if (is_valid) {
    debugPrint("QR Validation: SUCCESS");
    system_state.valid_scan = true;
    system_state.valid_delivery_sms_sent = false;
    displayLCD("Access Granted", "Opening locks...", "", "");
    logParcelHistory(qr_code, "VALIDATION_SUCCESS");

    openLock(1);
    delay(LOCK_OPERATION_DELAY);
    openLock(2);
    delay(LOCK_OPERATION_DELAY);

    playBuzzer("success");
    displayLCD("DOORS OPEN", "Place parcel in box", "Complete payment", "Door closes auto");
    Serial.println(F("[AUTH] Valid parcel - Locks opened"));
  } else {
    debugPrint("QR Validation: FAILED");
    system_state.valid_scan = false;
    playBuzzer("alert");
    displayLCD("Access Denied", "Invalid QR Code", "Try again", "");

    // — SMS: invalid count (3 consecutive failures) —
    system_state.invalid_scan_count++;
    debugPrint("Invalid scan count: " + String(system_state.invalid_scan_count));
    if (system_state.invalid_scan_count >= 3) {
      smsSendInvalidAttempt();
      system_state.invalid_scan_count = 0;  // reset after alert sent
    }

    logParcelHistory(qr_code, "VALIDATION_FAILED");
    delay(3000);
    displayLCD("READY", "Scan parcel QR",
               "WiFi: " + String(system_state.wifi_connected ? "OK" : "---"),
               "FB: " + String(system_state.firebase_connected ? "OK" : "---"));
  }
}

// ============================================================================
// HEALTH
// ============================================================================
void checkSystemHealth() {
  // Only print health if something changed
  Serial.println(F("\n=== System Health ==="));
  Serial.println("Uptime: " + String(millis() / 1000) + "s");
  Serial.println("WiFi: " + String(system_state.wifi_connected ? "Connected" : "Disconnected"));
  Serial.println("Firebase: " + String(system_state.firebase_connected ? "Connected" : "Disconnected"));
  Serial.println("Lock 1: " + String(system_state.lock1_open ? "OPEN" : "CLOSED"));
  Serial.println("Lock 2: " + String(system_state.lock2_open ? "OPEN" : "CLOSED"));
  Serial.println("Door 1: " + String(system_state.door1_open ? "OPEN" : "CLOSED"));
  Serial.println("Door 2: " + String(system_state.door2_open ? "OPEN" : "CLOSED"));
  Serial.println("=====================\n");
}

void closeLocksAfterDelivery() {
  debugPrint("Closing locks...");
  closeLock(1); delay(LOCK_OPERATION_DELAY);
  closeLock(2);
}

void emergencyLockdown() {
  debugPrint("EMERGENCY LOCKDOWN!");
  displayLCD("LOCKDOWN", "System Secured", "Contact Admin", "");
  closeLock(1); delay(LOCK_OPERATION_DELAY); closeLock(2);
  playBuzzer("alert");
  logParcelHistory("SYSTEM", "EMERGENCY_LOCKDOWN");
}

void resetSystem() {
  system_state.current_parcel_id = "";
  system_state.current_qr_code = "";
  system_state.current_receiver_phone = "";
  system_state.current_receiver_name = "";
  system_state.lock1_open = false;
  system_state.lock2_open = false;
  system_state.valid_scan = false;
  system_state.valid_delivery_sms_sent = false;
  system_state.invalid_scan_count = 0;
  system_state.door_breach_alerted = false;
  closeLocksAfterDelivery();
  displayLCD("SYSTEM RESET", "Ready for next parcel", "", "");
}

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