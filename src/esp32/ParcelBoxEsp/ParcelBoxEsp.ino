/*
 * ========================================
 * Smart Parcel Locker - ESP32
 * ========================================
 * 
 * Responsibilities:
 * 1. WiFi connectivity for cloud integration
 * 2. QR code scanner interface via UART
 * 3. I2C LCD display for status feedback
 * 4. Central logic and coordination
 * 5. Send commands to Arduino Uno for hardware control
 * 
 * Hardware Connections:
 * - QR Scanner RX: GPIO16 (Serial2 RX)
 * - QR Scanner TX: GPIO17 (Serial2 TX)
 * - I2C LCD SDA: GPIO21 (I2C address: 0x27)
 * - I2C LCD SCL: GPIO22
 * - Arduino Uno RX: GPIO3 (Serial RX)
 * - Arduino Uno TX: GPIO1 (Serial TX)
 */

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============ PIN & COMMUNICATION DEFINITIONS ============
#define SCL_PIN 22
#define SDA_PIN 21
#define QR_SCANNER_RX 16
#define QR_SCANNER_TX 17

#define BAUD_ESP32_UART_ARDUINO 115200  // Serial0 to Arduino Uno
#define BAUD_QR_SCANNER 9600             // Serial2 for QR Scanner

// ============ I2C LCD (20x4) ============
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Address 0x27, 20x4 display

// ============ WIFI CONFIGURATION ============
const char* WIFI_SSID = "YOUR_SSID";           // Change this
const char* WIFI_PASSWORD = "YOUR_PASSWORD";    // Change this
const char* API_ENDPOINT = "http://api.parcelbox.local/api";  // Backend API

// ============ STATE VARIABLES ============
struct SystemState {
  String parcel_id = "";
  String scanner_qr_code = "";
  bool lock1_open = false;
  bool lock2_open = false;
  bool door1_status = false;
  bool door2_status = false;
  unsigned long last_scan_time = 0;
  bool valid_scan = false;
} system_state;

// ============ FUNCTION DECLARATIONS ============
void setup();
void loop();
void setupWiFi();
void setupI2C_LCD();
void processQRScanner();
void displayLCD(String line1, String line2, String line3, String line4);
void sendCommandToArduino(String command);
void processArduinoResponse();
void handleParcelScanned(String qr_code);
void validateAndOpenLocks(String qr_code);
void debugPrint(String msg);

// ============ SETUP ============
void setup() {
  // Initialize serial communication
  Serial.begin(BAUD_ESP32_UART_ARDUINO);      // Hardware UART0 to Arduino
  Serial2.begin(BAUD_QR_SCANNER, SERIAL_8N1, QR_SCANNER_RX, QR_SCANNER_TX);
  
  // Initialize I2C & LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  setupI2C_LCD();
  
  // Display startup message
  displayLCD("PARCEL LOCKER", "Initializing...", "", "");
  delay(1000);
  
  debugPrint("ESP32 Starting...");
  
  // Setup WiFi
  setupWiFi();
  
  // Display ready state
  displayLCD("READY", "Waiting for QR...", "", "");
  
  debugPrint("ESP32 Ready!");
}

// ============ MAIN LOOP ============
void loop() {
  // Process QR code scanner input
  processQRScanner();
  
  // Process responses from Arduino
  processArduinoResponse();
  
  // Periodically check system health
  static unsigned long last_health_check = 0;
  if (millis() - last_health_check > 30000) {  // Every 30 seconds
    last_health_check = millis();
    checkSystemHealth();
  }
  
  delay(100);
}

// ============ WIFI SETUP ============
void setupWiFi() {
  debugPrint("Connecting to WiFi: " + String(WIFI_SSID));
  displayLCD("Connecting WiFi", WIFI_SSID, "...", "");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int connection_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && connection_attempts < 20) {
    delay(500);
    debugPrint(".");
    connection_attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    debugPrint("\nWiFi Connected!");
    debugPrint("IP: " + WiFi.localIP().toString());
    displayLCD("WiFi Connected", "IP: " + WiFi.localIP().toString(), "", "");
    delay(2000);
  } else {
    debugPrint("\nWiFi Connection Failed!");
    displayLCD("WiFi Failed", "Check credentials", "", "");
    delay(2000);
  }
}

// ============ I2C LCD SETUP ============
void setupI2C_LCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  debugPrint("LCD Initialized");
}

// ============ LCD DISPLAY FUNCTION ============
void displayLCD(String line1, String line2, String line3, String line4) {
  lcd.clear();
  
  // Truncate lines to 20 characters max
  if (line1.length() > 20) line1 = line1.substring(0, 20);
  if (line2.length() > 20) line2 = line2.substring(0, 20);
  if (line3.length() > 20) line3 = line3.substring(0, 20);
  if (line4.length() > 20) line4 = line4.substring(0, 20);
  
  lcd.setCursor(0, 0);
  lcd.print(line1);
  
  if (line2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  
  if (line3.length() > 0) {
    lcd.setCursor(0, 2);
    lcd.print(line3);
  }
  
  if (line4.length() > 0) {
    lcd.setCursor(0, 3);
    lcd.print(line4);
  }
}

// ============ QR SCANNER PROCESSING ============
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

// ============ PARCEL HANDLING WORKFLOW ============
void handleParcelScanned(String qr_code) {
  system_state.scanner_qr_code = qr_code;
  system_state.last_scan_time = millis();
  
  // Display scanning feedback
  displayLCD("QR SCANNED", qr_code, "Validating...", "");
  
  // Validate QR code with backend
  validateAndOpenLocks(qr_code);
}

void validateAndOpenLocks(String qr_code) {
  // TODO: Implement API call to validate QR code with backend
  // For now, simulate validation
  
  debugPrint("Validating QR: " + qr_code);
  
  // Mock validation - replace with actual API call
  bool is_valid = validateQRWithBackend(qr_code);
  
  if (is_valid) {
    debugPrint("QR Validation: SUCCESS");
    system_state.valid_scan = true;
    
    displayLCD("Access Granted", "Opening locks...", "", "");
    
    // Open parcel door (Lock #1)
    sendCommandToArduino("AT+LOCK1,OPEN");
    delay(500);
    
    // Open payment box (Lock #2)
    sendCommandToArduino("AT+LOCK2,OPEN");
    delay(500);
    
    // Success feedback
    sendCommandToArduino("AT+BUZZ,SUCCESS");
    displayLCD("PARCEL DOOR OPEN", "Place parcel inside", "Payment box open", "");
    
  } else {
    debugPrint("QR Validation: FAILED");
    system_state.valid_scan = false;
    
    // Access denied
    sendCommandToArduino("AT+BUZZ,ALERT");
    displayLCD("Access Denied", "Invalid QR Code", "Try again", "");
    delay(3000);
    displayLCD("READY", "Waiting for QR...", "", "");
  }
}

// ============ ARDUINO COMMUNICATION ============
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
        debugPrint("Parcel door closed - marking as delivered");
        displayLCD("Parcel Locked", "Delivery Complete", "", "");
        delay(2000);
        displayLCD("READY", "Waiting for QR...", "", "");
        
        // TODO: Send notification to backend
      }
      else if (event == "DOOR2_CLOSED") {
        system_state.door2_status = false;
        debugPrint("Payment box closed");
      }
    }
  }
}

// ============ BACKEND VALIDATION (MOCK) ============
bool validateQRWithBackend(String qr_code) {
  if (WiFi.status() != WL_CONNECTED) {
    debugPrint("WiFi not connected - validation failed");
    return false;
  }
  
  // TODO: Implement actual HTTP request to backend API
  // For development, accepting all QR codes >= 5 characters
  
  if (qr_code.length() >= 5) {
    system_state.parcel_id = qr_code;
    return true;
  }
  
  return false;
}

// ============ SYSTEM HEALTH CHECK ============
void checkSystemHealth() {
  debugPrint("\n=== System Health Check ===");
  debugPrint("WiFi Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
  debugPrint("Parcel ID: " + (system_state.parcel_id.length() > 0 ? system_state.parcel_id : "None"));
  debugPrint("Lock 1: " + String(system_state.lock1_open ? "OPEN" : "CLOSED"));
  debugPrint("Lock 2: " + String(system_state.lock2_open ? "OPEN" : "CLOSED"));
  debugPrint("===========================\n");
}

// ============ UTILITY FUNCTIONS ============
void debugPrint(String msg) {
  Serial.print("[ESP32] ");
  Serial.println(msg);
}

// ============ ADDITIONAL FUNCTIONS ============
/*
 * closeLocksAfterDelivery:
 * Called when delivery is confirmed (parcel door closes + payment collected)
 */
void closeLocksAfterDelivery() {
  debugPrint("Closing locks after delivery...");
  sendCommandToArduino("AT+LOCK1,CLOSE");
  delay(300);
  sendCommandToArduino("AT+LOCK2,CLOSE");
}

/*
 * emergencyLockdown:
 * Close all locks in case of unauthorized access or emergency
 */
void emergencyLockdown() {
  debugPrint("EMERGENCY LOCKDOWN ACTIVATED!");
  displayLCD("LOCKDOWN", "System Secured", "", "");
  sendCommandToArduino("AT+LOCK1,CLOSE");
  delay(300);
  sendCommandToArduino("AT+LOCK2,CLOSE");
  sendCommandToArduino("AT+BUZZ,ALERT");
}

/*
 * resetSystem:
 * Reset to initial state for next parcel
 */
void resetSystem() {
  system_state.parcel_id = "";
  system_state.scanner_qr_code = "";
  system_state.lock1_open = false;
  system_state.lock2_open = false;
  system_state.valid_scan = false;
  
  closeLocksAfterDelivery();
  displayLCD("READY", "Waiting for QR...", "", "");
}
