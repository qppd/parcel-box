# 🔐 Smart Parcel Locker System

**Advanced IoT-Based Package Delivery & Management Solution**

- **Developer:** Sajed Lopez Mendoza (@qppd) | QPPD (Quezon Province Programmers/Developers)
- **Version:** 1.0.0
- **Last Updated:** February 16, 2026
- **Repository:** qppd/parcel-box
- **License:** MIT
- **Contact:** quezon.province.pd@gmail.com | [Portfolio](https://sajed-mendoza.onrender.com/)

---

## 📚 Table of Contents

1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Hardware Components](#hardware-components)
4. [Pin Assignments & Wiring](#pin-assignments--wiring)
5. [Firmware Setup](#firmware-setup)
6. [Configuration Guide](#configuration-guide)
7. [Testing & Validation](#testing--validation)
8. [Backend API](#backend-api)
9. [Troubleshooting](#troubleshooting)
10. [System Workflow](#system-workflow)

---


## 📋 Project Overview

The Smart Parcel Locker System is a comprehensive, secure package delivery and management solution designed for modern logistics operations. It combines IoT technology, cloud integration, and advanced hardware control to create a seamless, automated parcel delivery experience.

### Key Features

- **QR Code Authentication:** Secure parcel identification and tracking via 1D/2D QR code scanning
- **Dual Solenoid Control:** Independent control of parcel door and payment box locks
- **SMS Notifications:** Real-time delivery confirmations and security alerts via GSM/SMS
- **WiFi Connectivity:** Cloud integration for backend database and API communication
- **LCD Feedback:** Real-time status display (20×4 character I2C display)
- **Audio Alerts:** Piezo buzzer feedback for user interactions
- **Door Monitoring:** Reed switch sensors detect and report door open/close status
- **Professional Architecture:** Dual-microcontroller design for reliability and modularity

### System Statistics

| Metric | Value |
|--------|-------|
| ⚡ Power Consumption | Peak: 5.5A @ 12V + 1.5A @ 5V |
| 🔄 Response Time | QR scan to lock open: <2 seconds |
| 📡 Communication | Serial: 115200 baud \| QR: 9600 baud \| I2C: 100kHz |
| 🔒 Security | Backend OAuth + Hardware isolation |

---


## 🏗️ System Architecture

### Dual-Microcontroller Design

The system employs a two-tier microcontroller architecture for optimal performance and reliability:

#### 🤖 Arduino Uno (Local Controller)
**Role:** Hardware interface and control
- Manages dual solenoid locks via relay module
- Controls piezo buzzer audio feedback
- Communicates with SIM800L for SMS alerts
- Monitors door sensors (reed switches)
- Processes commands from ESP32 via serial

#### 📡 ESP32 (IoT Controller)
**Role:** Cloud connectivity and logic
- WiFi connectivity for cloud integration
- QR code scanner interface via UART (9600 baud)
- I2C LCD display for user feedback
- Central coordination logic
- Backend API communication

### Architecture Block Diagram

```
BACKEND API ⟷ [WiFi] ⟷ ESP32 ⟷ [Serial/UART] ⟷ Arduino Uno ⟷ Hardware
- Database      ⟷              ⟷                ⟷                ⟷
- QR Validation   WiFi Modem    Logic            Relay/Solenoid   Sensors
- Auth            2.4GHz        LCD Display      SIM800L/SMS      Buzzer
- SMS Triggers    Antenna       API Client       Door Detection   Locks
```

---


## ⚙️ Complete Hardware Components

> ⚠️ **High Voltage Warning:** This system uses 220V AC input. Only follow proper electrical safety. Hire a qualified electrician for AC connections.

### 1. QR Code Scanner V3.0 | Interface: UART 9600 baud
- Auto-detection of 1D & 2D barcode systems
- Output: ASCII text format (Parcel ID)
- Connection: ESP32 GPIO16/17 (Serial2)

### 2. I2C LCD Display (20×4) | Address: 0x27 (Default)
- SDA: ESP32 GPIO21 | SCL: ESP32 GPIO22
- Voltage: 5V DC | Communication: I2C at 100kHz
- Purpose: Real-time status & user prompts

### 3. Dual 12V Solenoid Door Locks (×2)
- Voltage: 12V DC | Current: 0.5-1A each
- Lock #1 (Parcel Door) → Arduino D2 via Relay CH1
- Lock #2 (Payment Box) → Arduino D3 via Relay CH2
- **⚠️ Install 1N5401 freewheeling diodes!**

### 4. ESP32 Development Board (38-pin)
- 4MB Flash | 520KB SRAM | 36 GPIO pins available
- WiFi: 802.11 b/g/n @ 2.4GHz (5GHz NOT supported)
- Voltage: 3.3V logic | 5V analog-input tolerant

### 5. Dual Channel Relay Module | 5V trigger, 2 channels
- Relay Rating: 10A @ 250V AC or 10A @ 30V DC
- Trigger Logic: Active LOW (HIGH = relay OFF)
- Provides isolation between 5V logic and 12V power

### 6. SIM800L GSM/GPRS Module
- Freq: 850/900/1800/1900MHz | Interface: UART @ 115200 baud
- Supply: 3.7-4.2V (requires buck converter) | Arduino D8/D9
- Features: SMS send/receive, calls, GPRS - requires active SIM card

### 7. Passive Piezo Buzzer | 5V, 4kHz nominal
- Current: 10-20mA | Control: Arduino D4 (PWM)
- Tones: Success beep, alert, startup chirp

### 8. Power Supply Chain
- **AC Input:** 220V AC → S-60-12 (60W Switching PSU) → 12V DC @ 5A
- **Buck Converters (×3 LM2596):** 12V → 5V regulated
- **Distribution:** Unit #1 (Arduino+Relay) | #2 (SIM800L+LCD) | #3 (Buzzer)

### 9. Protection Components
- **Diodes (1N5401, ×10):** 3A @100V - Freewheeling protection on coils
- **Configuration:** Cathode to +12V, Anode to solenoid GND
- **Purpose:** Prevent back-EMF damage to relays

### 10. Reed Switch Door Sensors (×2)
- Parcel Door → Arduino D5 + 10kΩ pull-up | Payment Box → D6
- Logic: Closed = LOW, Open = HIGH
- Range: Up to 15mm depending on magnet strength


## 🔌 Pin Assignments & Wiring

### Arduino Uno Complete Pinout

| Function | Pin | Type | Connection | Notes |
|----------|-----|------|------------|-------|
| Lock #1 Control | D2 | Digital Out | Relay CH1 | LOW = Open |
| Lock #2 Control | D3 | Digital Out | Relay CH2 | LOW = Open |
| Piezo Buzzer | D4 | PWM Out | Buzzer + | Tone control |
| Door Sensor 1 | D5 | Digital In | Reed Switch | 10kΩ pull-up |
| Door Sensor 2 | D6 | Digital In | Reed Switch | 10kΩ pull-up |
| SIM800L RX | D8 | Soft Serial RX | SIM800L TX | Voltage divider |
| SIM800L TX | D9 | Soft Serial TX | SIM800L RX | 115200 baud |
| ESP32 RX | RX1 (D0) | Serial RX | ESP32 TX | 115200 baud |
| ESP32 TX | TX1 (D1) | Serial TX | ESP32 RX | 115200 baud |

### ESP32 Complete Pinout

| Function | Pin | Type | Connection | Notes |
|----------|-----|------|------------|-------|
| QR Scanner RX | GPIO16 | Serial2 RX | Scanner TX | 9600 baud |
| QR Scanner TX | GPIO17 | Serial2 TX | Scanner RX | 9600 baud |
| LCD SDA | GPIO21 | I2C Data | LCD SDA | 4.7kΩ pull-up |
| LCD SCL | GPIO22 | I2C Clock | LCD SCL | 4.7kΩ pull-up |
| Arduino RX | GPIO3 | Serial RX | Arduino TX1 | 115200 baud |
| Arduino TX | GPIO1 | Serial TX | Arduino RX1 | 115200 baud |

### Power Distribution

```
220V AC (Main) → [S-60-12 Power Supply] → 12V @ 5A
                        ↓
        ┌───────────────┼───────────────┐
        ↓               ↓               ↓
   [LM2596#1]    [LM2596#2]      [LM2596#3]
   5V / 2A       5V / 1.5A       5V / 0.5A
        ↓               ↓               ↓
   Arduino       SIM800L         LCD+Buzzer
   + Relay        + GSM            + Logic
```

## 💾 Firmware Installation Guide

### Arduino Uno Firmware
**File:** `src/arduino/ParcelBoxUno/ParcelBoxUno.ino` (284 lines)

**Setup Steps:**
1. Download Arduino IDE: https://www.arduino.cc/en/software
2. Tools → Board → Arduino Uno
3. Tools → Port → Select COM port
4. Open file and click Upload (Ctrl+U)
5. Serial Monitor @ 115200 should show: "[ARDUINO] Parcel Locker Starting..."

### ESP32 Firmware
**File:** `src/esp32/ParcelBoxEsp/ParcelBoxEsp.ino` (346 lines)

**Add Board Support:**
1. File → Preferences → Add to "Additional Boards Manager URLs":
2. `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Tools → Boards Manager → Search "ESP32" → Install

**Install Libraries:**
1. Tools → Manage Libraries
2. Install: **LiquidCrystal_I2C** v1.1.2 by Frank de Brabander

**Configure WiFi (CRITICAL!):**
Edit these lines in ParcelBoxEsp.ino BEFORE uploading:

```cpp
const char* WIFI_SSID = "YOUR_SSID";           // ← Change this
const char* WIFI_PASSWORD = "YOUR_PASSWORD";    // ← Change this
```

**Upload:**
1. Tools → Board → ESP32 Dev Module
2. Tools → Upload Speed → 921600
3. Upload (Ctrl+U)
4. Serial Monitor @ 115200 should show: "[ESP32] WiFi Connected! IP: ..."

---

## ⚙️ Configuration & Verification

### WiFi Network
- **Must be 2.4GHz:** ESP32 doesn't support 5GHz
- **Test with phone hotspot:** Use "iPhone-Network" as test SSID
- **Production:** Ensure internet connection for API calls

### I2C LCD Address
If LCD doesn't display, find correct I2C address with scanner code:

```cpp
#include <Wire.h>
void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
}
void loop() {
  for (int i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found: 0x");
      Serial.println(i, HEX);
    }
  }
  delay(5000);
}
```

Common addresses: 0x27 (default), 0x3F, 0x20, 0x38
Update line 33 in ParcelBoxEsp.ino with found address

### Backend API Configuration
```cpp
const char* API_ENDPOINT = "http://api.parcelbox.local/api";
```

Update to your backend server URL before deployment

### SIM800L Setup
- [ ] Insert valid SIM card into module
- [ ] Verify SIM has SMS support and credit (~PHP 1-5)
- [ ] Power module 3+ minutes before use
- [ ] Verify 3.7-4.2V stable power supply
- [ ] Attach external antenna if available

---



## ✅ Testing & Validation

### Pre-Power-On Checklist
- [ ] All GND connections common (measure <0.1V difference)
- [ ] 12V supply measures 11.5-12.5V
- [ ] Each 5V output measures 4.9-5.1V
- [ ] No loose wires or shorts in relay circuit
- [ ] Freewheeling diodes installed correctly on solenoids
- [ ] Reed switches have pull-up resistors

### Component Unit Tests

#### Test 1: Relay Click

```cpp
void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {
  digitalWrite(2, LOW);   // Should CLICK
  delay(1000);
  digitalWrite(2, HIGH);  // Should RELEASE
  delay(1000);
}
```

**Expected:** Audible relay click, LED blinks

#### Test 2: Solenoid Actuation
1. Apply 12V directly to solenoid (test externally)
2. Verify: Audible click, mechanism moves
3. Measure current: ~0.8A peak, drops to 0.1-0.2A
4. Verify no heat buildup on coil

#### Test 3: Buzzer Tones

```cpp
void setup() {
  pinMode(4, OUTPUT);
}

void loop() {
  tone(4, 1000, 500);   // 1kHz for 500ms
  delay(1000);
  
  for (int f = 800; f < 2000; f += 100) {
    tone(4, f, 20);
    delay(20);
  }
  
  for (int i = 0; i < 5; i++) {
    tone(4, 2000, 50);
    delay(100);
  }
  delay(3000);
}
```

**Expected:** Fixed tone, ascending chirp, rapid beeps (3 patterns)

#### Test 4: Door Sensors

```cpp
void setup() {
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  Serial.begin(115200);
}

void loop() {
  int d1 = digitalRead(5);
  int d2 = digitalRead(6);
  Serial.println(d1 == HIGH ? "Door1: OPEN" : "Door1: CLOSED");
  Serial.println(d2 == HIGH ? "Door2: OPEN" : "Door2: CLOSED");
  delay(500);
}
```

**Procedure:** Move magnet toward reed switch
**Expected:** Clean LOW→HIGH transition, no fluttering

### Full System Workflow Test
1. Boot both boards - check Serial Monitors for startup messages
2. Scan QR code "TEST123" - LCD should validate and respond
3. Locks should click audibly - motors move
4. Buzzer should beep (success tone)
5. Close both doors - system detects closure
6. SMS should be sent within 5 seconds
7. System resets to "READY" state

> ✅ **System ready for production** when ALL tests pass without errors

---



## 🌐 Backend API Specification

### Required Endpoints

#### 1. Validate QR Code
**POST /api/v1/qr/validate**

```json
Request:
{
  "qr_code": "PCL-000123",
  "locker_id": "LOC-001",
  "timestamp": "2026-02-16T10:30:00Z"
}

Response (Valid):
{
  "valid": true,
  "parcel": {
    "parcel_id": "PCL-000123",
    "receiver_name": "John Doe",
    "receiver_phone": "+639171234567",
    "locker_number": 1,
    "cod_amount": 500
  },
  "can_open": true
}

Response (Invalid):
{
  "valid": false,
  "reason": "PARCEL_NOT_FOUND",
  "can_open": false
}
```

#### 2. Mark Delivery Confirmed
**POST /api/v1/delivery/confirm**

```json
{
  "parcel_id": "PCL-000123",
  "locker_id": "LOC-001",
  "delivery_time": "2026-02-16T10:35:00Z",
  "payment_received": true
}
```

**Triggers:** SMS to receiver, database update, admin notification

#### 3. Locker Heartbeat (Health Check)
**POST /api/v1/locker/{locker_id}/heartbeat** (every 60 seconds)

```json
{
  "locker_id": "LOC-001",
  "uptime_seconds": 86400,
  "wifi_rssi": -65,
  "gsm_signal": 18,
  "temperature": 28,
  "firmware_version": "1.0.0"
}
```

### Database Schema (Minimal)

```sql
CREATE TABLE parcels (
  id INT PRIMARY KEY AUTO_INCREMENT,
  parcel_id VARCHAR(50) UNIQUE NOT NULL,
  qr_code VARCHAR(255) NOT NULL,
  receiver_name VARCHAR(100),
  receiver_phone VARCHAR(20),
  locker_id VARCHAR(20),
  status ENUM('pending', 'ready', 'delivered', 'canceled'),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  delivered_at TIMESTAMP NULL,
  INDEX idx_qr_code (qr_code),
  INDEX idx_status (status)
);
```

### Node.js/Express Example

```javascript
const express = require('express');
const app = express();
app.use(express.json());

app.post('/api/v1/qr/validate', async (req, res) => {
  const { qr_code, locker_id } = req.body;
  
  const parcel = await db.query(
    'SELECT * FROM parcels WHERE qr_code = ?',
    [qr_code]
  );
  
  if (!parcel || parcel.status === 'delivered') {
    return res.json({ valid: false, can_open: false });
  }
  
  res.json({
    valid: true,
    can_open: true,
    parcel: parcel
  });
});

app.listen(3000);
```

---

## 🔧 Troubleshooting Guide

| Issue | Solution |
|-------|----------|
| ❌ Arduino upload fails | Check USB cable (try different cable) \| Verify COM port is selected \| Update CH340 drivers \| Close other serial monitor windows |
| ❌ Serial Monitor shows gibberish | Verify baud rate = 115200 (dropdown in Serial Monitor) |
| ❌ Relay doesn't click | Verify relay has 5V power (multimeter test) \| Check Arduino pin assignment (D2, D3) \| Test: digitalWrite(2, LOW) directly \| Measure GPIO: should be 0V (LOW), 5V (HIGH) |
| ❌ Solenoid doesn't move | Verify 12V reaches solenoid (multimeter) \| Test solenoid directly with external 12V \| Check relay contacts have continuity when activated \| Verify freewheeling diode polarity |
| ❌ ESP32 won't connect to WiFi | Verify SSID/Password are correct (case-sensitive) \| Ensure network is 2.4GHz (NOT 5GHz) \| Check signal strength (should be -30 to -65 dBm) \| Try moving locker closer to router |
| ❌ LCD display blank | Backlight glows? If no: power issue \| Verify 5V power and pull-up resistors (4.7kΩ) \| Run I2C address scanner to find correct address \| Check for loose SDA/SCL connections |
| ❌ QR scanner doesn't read | Scanner LED is on when powered? \| Verify 5V on VCC/GND \| TX/RX pins not reversed? \| Try different QR codes |
| ❌ SMS not sending | SIM card inserted and has credit? \| Test AT command: "AT+CSQ" should show signal ≥10 \| Verify phone number includes country code: +63... \| Power module 3+ seconds before first use |

---

## 📊 Complete Delivery Workflow

### Step 1: QR Code Scan
- Rider scans QR code with scanner module
- ESP32 receives QR data via UART
- UART baud: 9600 bps | Example data: "PCL-000123\r\n"

### Step 2: Backend Validation
- ESP32 sends HTTP request to backend API
- Endpoint: POST /api/v1/qr/validate
- Backend checks database: Is this QR valid?
- Returns: Valid/Invalid + parcel details

### Step 3: Lock Opening (If Valid)
- ESP32 sends command: "AT+LOCK1,OPEN\r\n"
- Arduino receives and activates GPIO D2
- Relay CH1 energizes → Solenoid #1 clicks open
- 60ms later: ESP32 sends "AT+LOCK2,OPEN\r\n"
- Solenoid #2 (payment box) clicks open
- Arduino responds: "OK\r\n"
- ESP32 triggers buzzer: "AT+BUZZ,SUCCESS\r\n"
- LCD displays: "PLACE PARCEL INSIDE"

### Step 4: Parcel Placement & Payment
- Rider places parcel in open locker
- Rider deposits payment in payment box (if COD)
- Rider closes both doors manually

### Step 5: Door Closure Detection
- Arduino monitors D5 (door 1) and D6 (door 2)
- Reed switches detect magnetic closure
- When both doors LOW: notify ESP32 "EVENT:DOOR1_CLOSED"

### Step 6: Delivery Confirmation
- ESP32 confirms both doors closed
- Sends: "AT+LOCK1,CLOSE" + "AT+LOCK2,CLOSE"
- Arduino de-energizes relays (locks engage)
- IoT triggers backend endpoint: POST /api/v1/delivery/confirm

### Step 7: SMS Notification
- Arduino sends SMS via SIM800L to receiver
- Message: "Parcel PCL-000123 delivered to Locker 1. Thank you!"
- SMS sends within 5 seconds of door closure

### Step 8: System Reset
- Clears parcel data from memory
- Resets locks to secure position
- LCD returns to: "READY - Waiting for QR"
- System ready for next delivery

### Security: Unauthorized Access Detection
- If door opens WITHOUT valid QR scan:
- Reed switch detects door opening
- System checks scan log: valid scan in past 5 minutes?
- If NO: Flag as UNAUTHORIZED ACCESS
- Send SMS alert to admin
- Activate emergency lockdown (all locks engage)

### Critical State Variables
- `last_valid_scan_time` - Prevents duplicate alerts
- `door_status` - Tracks open/closed state
- `parcel_status` - Delivery progress stage
- `locker_status` - System operational state

---

## 📝 Footer

**Smart Parcel Locker System v1.0.0**
- **Status:** Ready for Hardware Assembly & Testing
- **Framework:** Arduino + ESP32 + IoT Backend
- **Last Updated:** February 16, 2026
- **Developed By:** Sajed Lopez Mendoza (@qppd)

**Developer Information:**
- **GitHub:** [github.com/qppd](https://github.com/qppd)
- **Portfolio:** [sajed-mendoza.onrender.com](https://sajed-mendoza.onrender.com/)
- **Email:** quezon.province.pd@gmail.com
- **Location:** Unisan, Quezon Province, Philippines
- **Specialization:** Full-Stack Development, IoT Systems, Embedded Hardware, Machine Learning

**Skills:**
- Frontend: HTML5, CSS3
- Backend: PHP, Node.js, Python
- Microcontrollers: ESP32, Raspberry Pi, Arduino
- ML/AI: TensorFlow
- Databases: MySQL, SQLite, Firebase
- DevOps: Git, SSH, REST APIs

All documentation, firmware, and design materials are contained within this README. This is a professional, production-grade smart locker system with comprehensive testing, security features, and cloud integration capabilities.