/*
 * ========================================
 * Smart Parcel Locker - Arduino Uno
 * ========================================
 * 
 * Responsibilities:
 * 1. Control dual solenoid locks via relay module
 * 2. Monitor door sensors (reed switches)
 * 3. Control piezo buzzer feedback
 * 4. Communicate with SIM800L for SMS alerts
 * 5. Receive commands from ESP32 via Serial
 * 
 * Hardware Connections:
 * - Solenoid Lock #1: D2 (Relay CH1)
 * - Solenoid Lock #2: D3 (Relay CH2)
 * - Piezo Buzzer: D4
 * - Reed Switch Door #1 (Parcel): D5
 * - Reed Switch Door #2 (Payment Box): D6
 * - ESP32 RX: RX1 (D0)
 * - ESP32 TX: TX1 (D1)
 * - SIM800L RX: D8 (Software Serial)
 * - SIM800L TX: D9 (Software Serial)
 */

#include <SoftwareSerial.h>

// ============ PIN DEFINITIONS ============
#define LOCK1_PIN 2              // Solenoid Lock #1 Control
#define LOCK2_PIN 3              // Solenoid Lock #2 Control
#define BUZZER_PIN 4             // Piezo Buzzer (PWM-capable)
#define DOOR_SENSOR1_PIN 5       // Reed Switch - Parcel Door
#define DOOR_SENSOR2_PIN 6       // Reed Switch - Payment Box

// ============ SERIAL DEFINITIONS ============
#define ESP32_BAUD 115200        // ESP32 Communication (Hardware Serial1)
#define SIM800L_BAUD 115200      // SIM800L Communication
SoftwareSerial sim800l(8, 9);    // RX, TX pins for SIM800L

// ============ STATE VARIABLES ============
volatile bool door1_open = false;
volatile bool door2_open = false;
unsigned long last_sms_time = 0;
const unsigned long SMS_COOLDOWN = 3000; // Prevent SMS spam (3s)

// ============ FUNCTION DECLARATIONS ============
void setup();
void loop();
void processESP32Commands();
void openLock(int lockNum);
void closeLock(int lockNum);
void playBuzzer(String tone_type);
void checkDoorSensors();
void sendSMS(String phone, String message);
void initSIM800L();
void debugPrint(String msg);

// ============ SETUP ============
void setup() {
  // Initialize serial communication
  Serial.begin(ESP32_BAUD);        // ESP32 communication (Hardware Serial1)
  sim800l.begin(SIM800L_BAUD);    // SIM800L communication (Software Serial)
  
  // Initialize pins
  pinMode(LOCK1_PIN, OUTPUT);
  pinMode(LOCK2_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(DOOR_SENSOR1_PIN, INPUT);
  pinMode(DOOR_SENSOR2_PIN, INPUT);
  
  // Set initial states (locks closed = HIGH due to relay logic)
  digitalWrite(LOCK1_PIN, HIGH);
  digitalWrite(LOCK2_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Startup sequence
  debugPrint("Parcel Locker - Arduino Uno Starting...");
  delay(1000);
  
  // Initialize SIM800L
  initSIM800L();
  
  // Startup beep
  playBuzzer("Startup");
  
  debugPrint("Arduino Uno Ready!");
}

// ============ MAIN LOOP ============
void loop() {
  // Process incoming commands from ESP32
  processESP32Commands();
  
  // Check door sensors
  checkDoorSensors();
  
  // Drain SIM800L buffer to prevent overflow
  while (sim800l.available()) {
    sim800l.read();
  }
  
  delay(50);
}

// ============ ESP32 COMMAND PROCESSOR ============
void processESP32Commands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.replace("\r", "");  // Remove carriage return
    
    debugPrint("Received: " + command);
    
    // Parse and execute commands
    if (command == "AT+LOCK1,OPEN") {
      openLock(1);
      Serial.println("OK");
    }
    else if (command == "AT+LOCK2,OPEN") {
      openLock(2);
      Serial.println("OK");
    }
    else if (command == "AT+LOCK1,CLOSE") {
      closeLock(1);
      Serial.println("OK");
    }
    else if (command == "AT+LOCK2,CLOSE") {
      closeLock(2);
      Serial.println("OK");
    }
    else if (command == "AT+BUZZ,SUCCESS") {
      playBuzzer("success");
      Serial.println("OK");
    }
    else if (command == "AT+BUZZ,ALERT") {
      playBuzzer("alert");
      Serial.println("OK");
    }
    else if (command == "AT+DOOR,STATUS") {
      checkDoorSensors();
      Serial.print("DOOR1:");
      Serial.println(door1_open ? "OPEN" : "CLOSED");
      Serial.print("DOOR2:");
      Serial.println(door2_open ? "OPEN" : "CLOSED");
    }
    else {
      Serial.println("ERROR");
    }
  }
}

// ============ LOCK CONTROL FUNCTIONS ============
/*
 * Relay Module Logic:
 * - Signal LOW (0V) = Relay ACTIVE = Solenoid energized (lock opens)
 * - Signal HIGH (5V) = Relay INACTIVE = Solenoid de-energized (lock closes/locked)
 */
void openLock(int lockNum) {
  int pin = (lockNum == 1) ? LOCK1_PIN : LOCK2_PIN;
  digitalWrite(pin, LOW);  // Relay active → Solenoid open
  debugPrint("Lock " + String(lockNum) + " OPENED");
  playBuzzer("click");
}

void closeLock(int lockNum) {
  int pin = (lockNum == 1) ? LOCK1_PIN : LOCK2_PIN;
  digitalWrite(pin, HIGH);  // Relay inactive → Solenoid closed
  debugPrint("Lock " + String(lockNum) + " CLOSED");
}

// ============ BUZZER CONTROL ============
void playBuzzer(String tone_type) {
  if (tone_type == "startup") {
    // Startup: 2 short beeps
    for (int i = 0; i < 2; i++) {
      tone(BUZZER_PIN, 1000, 100);  // Freq: 1kHz, Duration: 100ms
      delay(150);
    }
  }
  else if (tone_type == "success") {
    // Success: ascending chirp (200ms)
    for (int freq = 800; freq < 2000; freq += 50) {
      tone(BUZZER_PIN, freq, 20);
      delay(20);
    }
  }
  else if (tone_type == "alert") {
    // Alert: rapid beeping (500ms)
    for (int i = 0; i < 5; i++) {
      tone(BUZZER_PIN, 2000, 50);
      delay(100);
    }
  }
  else if (tone_type == "click") {
    // Click: single short beep
    tone(BUZZER_PIN, 1500, 50);
    delay(100);
  }
  
  noTone(BUZZER_PIN);  // Ensure buzzer is off
}

// ============ DOOR SENSOR MONITORING ============
void checkDoorSensors() {
  // Read sensors (LOW = closed, HIGH = open with pull-up)
  bool new_door1_state = digitalRead(DOOR_SENSOR1_PIN) == HIGH;
  bool new_door2_state = digitalRead(DOOR_SENSOR2_PIN) == HIGH;
  
  // Detect state changes
  if (new_door1_state != door1_open) {
    door1_open = new_door1_state;
    if (door1_open) {
      debugPrint("Parcel door OPENED");
    } else {
      debugPrint("Parcel door CLOSED");
      // Notify ESP32
      Serial.println("EVENT:DOOR1_CLOSED");
    }
  }
  
  if (new_door2_state != door2_open) {
    door2_open = new_door2_state;
    if (door2_open) {
      debugPrint("Payment box door OPENED");
    } else {
      debugPrint("Payment box door CLOSED");
      Serial.println("EVENT:DOOR2_CLOSED");
    }
  }
}

// ============ SIM800L GSM FUNCTIONS ============
void initSIM800L() {
  debugPrint("Initializing SIM800L...");
  
  // Send basic AT command to verify module
  sim800l.println("AT");
  delay(500);
  
  // Flush any response
  while (sim800l.available()) {
    sim800l.read();
  }
  
  debugPrint("SIM800L initialized");
}

void sendSMS(String phone, String message) {
  // Implement cooldown to prevent SMS spam
  unsigned long current_time = millis();
  if (current_time - last_sms_time < SMS_COOLDOWN) {
    debugPrint("SMS cooldown active, skipping send");
    return;
  }
  
  debugPrint("Sending SMS to: " + phone);
  debugPrint("Message: " + message);
  
  // SIM800L AT Command sequence
  sim800l.println("AT+CMGF=1");  // Set text mode
  delay(100);
  
  // Set recipient number
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phone);
  sim800l.println("\"");
  delay(100);
  
  // Send message
  sim800l.print(message);
  sim800l.write(26);  // Ctrl+Z to send
  delay(1000);
  
  last_sms_time = current_time;
  
  // Confirm
  debugPrint("SMS sent!");
  playBuzzer("click");
}

// ============ DEBUG/LOGGING ============
void debugPrint(String msg) {
  Serial.print("[ARDUINO] ");
  Serial.println(msg);
}
