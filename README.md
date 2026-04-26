| 🔒 Security | Firebase Auth + ESP-NOW encryption |# 🔐 Smart Parcel Locker System
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Advanced IoT-Based Package Delivery & Management Solution**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Developer:** Sajed Lopez Mendoza (@qppd) | QPPD (Quezon Province Programmers/Developers)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Version:** 1.0.0
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Repository:** qppd/parcel-box
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **License:** MIT
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Contact:** quezon.province.pd@gmail.com | [Portfolio](https://sajed-mendoza.onrender.com/)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 📚 Table of Contents
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. [Project Overview](#project-overview)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. [System Architecture](#system-architecture)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. [Hardware Components](#hardware-components)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. [Pin Assignments & Wiring](#pin-assignments--wiring)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. [Firmware Setup](#firmware-setup)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |6. [Configuration Guide](#configuration-guide)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |7. [Testing & Validation](#testing--validation)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |8. [Backend API](#backend-api)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |9. [Troubleshooting](#troubleshooting)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |10. [System Workflow](#system-workflow)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 📋 Project Overview
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |The Smart Parcel Locker System is a comprehensive, secure package delivery and management solution designed for modern logistics operations. It combines IoT technology, cloud integration, and advanced hardware control to create a seamless, automated parcel delivery experience.
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Key Features
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **QR Code Authentication:** Secure parcel identification and tracking via 1D/2D QR code scanning
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Dual Solenoid Control:** Independent control of parcel door and payment box locks
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **SMS Notifications:** Real-time delivery confirmations and security alerts via GSM/SMS
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **WiFi Connectivity:** Cloud integration for backend database and API communication
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **LCD Feedback:** Real-time status display (20×4 character I2C display)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Wireless QR Scanning: ESP32-CAM camera-based QR scanner with ESP-NOW peer-to-peer wireless transmission
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Audio Alerts:** Piezo buzzer feedback for user interactions
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Door Monitoring:** Reed switch sensors detect and report door open/close status
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Professional Architecture:** Dual-microcontroller design for reliability and modularity
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### System Statistics
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Metric | Value |
| 🔒 Security | Firebase Auth + ESP-NOW encryption ||--------|-------|
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ⚡ Power Consumption | Peak: 5.5A @ 12V + 1.5A @ 5V |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || 🔄 Response Time | QR scan to lock open: <2 seconds |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || 📡 Communication | Serial: 115200 baud \| QR: 9600 baud \| I2C: 100kHz |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || 🔒 Security | Backend OAuth + Hardware isolation |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 🏗️ System Architecture
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Single-ESP32 with Wireless QR Scanner
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |The system employs a modern single-ESP32 architecture with wireless QR scanning via ESP-NOW:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 📡 Main ESP32 (38-pin) - Central Controller
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Role:** Hardware control, cloud connectivity, and coordination
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- WiFi connectivity for Firebase Realtime Database integration
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP-NOW receiver for wireless QR codes from ESP32-CAM
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- UART QR scanner interface (backup/secondary)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- I2C LCD display for user feedback
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Direct GPIO control of relay locks, buzzer, and door sensors
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- SIM800L GSM module for SMS notifications
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Central coordination and decision logic
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 📷 ESP32-CAM (AI-Thinker) - Wireless QR Scanner
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Role:** Dedicated QR code scanner with wireless transmission
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- OV2640 camera for QR code detection
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP-NOW transmitter (peer-to-peer, no WiFi needed)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Sends scanned QR codes wirelessly to main ESP32
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- LED indicator for scan/send status
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Standalone operation - does not require WiFi or cloud access
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 🤖 Legacy UART QR Scanner (Optional)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Role:** Backup QR scanning via hardware UART
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- External barcode scanner module (9600 baud)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- UART1 interface (GPIO33 RX / GPIO26 TX)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Active when ESP32-CAM unavailable
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Architecture Block Diagram
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |FIREBASE RTDB ⟷═╗
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Database      ⟷              ⟷                ⟷                ⟷
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- QR Validation   WiFi Modem    Logic            Relay/Solenoid   Sensors
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Auth            2.4GHz        LCD Display      SIM800L/SMS      Buzzer
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- SMS Triggers    Antenna       API Client       Door Detection   Locks
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## ⚙️ Complete Hardware Components
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |> ⚠️ **High Voltage Warning:** This system uses 220V AC input. Only follow proper electrical safety. Hire a qualified electrician for AC connections.
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 1. QR Code Scanner V3.0 | Interface: UART 9600 baud
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Auto-detection of 1D & 2D barcode systems
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Output: ASCII text format (Parcel ID)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Connection: ESP32 GPIO16/17 (Serial2)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 2. I2C LCD Display (20×4) | Address: 0x27 (Default)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- SDA: ESP32 GPIO21 | SCL: ESP32 GPIO22
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Voltage: 5V DC | Communication: I2C at 100kHz
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Purpose: Real-time status & user prompts
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 3. Dual 12V Solenoid Door Locks (×2)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Voltage: 12V DC | Current: 0.5-1A each
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Lock #1 (Parcel Door) → Arduino D2 via Relay CH1
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Lock #2 (Payment Box) → Arduino D3 via Relay CH2
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **⚠️ Install 1N5401 freewheeling diodes!**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 4. ESP32 Development Board (38-pin)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- 4MB Flash | 520KB SRAM | 36 GPIO pins available
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- WiFi: 802.11 b/g/n @ 2.4GHz (5GHz NOT supported)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Voltage: 3.3V logic | 5V analog-input tolerant
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 5. Dual Channel Relay Module | 5V trigger, 2 channels
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Relay Rating: 10A @ 250V AC or 10A @ 30V DC
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Trigger Logic: Active LOW (HIGH = relay OFF)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Provides isolation between 5V logic and 12V power
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 6. SIM800L GSM/GPRS Module
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Freq: 850/900/1800/1900MHz | Interface: UART @ 115200 baud
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Supply: 3.7-4.2V (requires buck converter) | Arduino D8/D9
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Features: SMS send/receive, calls, GPRS - requires active SIM card
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 7. Passive Piezo Buzzer | 5V, 4kHz nominal
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Current: 10-20mA | Control: Arduino D4 (PWM)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Tones: Success beep, alert, startup chirp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 8. Power Supply Chain
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **AC Input:** 220V AC → S-60-12 (60W Switching PSU) → 12V DC @ 5A
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Buck Converters (×3 LM2596):** 12V → 5V regulated
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Distribution:** Unit #1 (Arduino+Relay) | #2 (SIM800L+LCD) | #3 (Buzzer)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 9. Protection Components
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Diodes (1N5401, ×10):** 3A @100V - Freewheeling protection on coils
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Configuration:** Cathode to +12V, Anode to solenoid GND
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Purpose:** Prevent back-EMF damage to relays
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### 10. Reed Switch Door Sensors (×2)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Parcel Door → Arduino D5 + 10kΩ pull-up | Payment Box → D6
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Logic: Closed = LOW, Open = HIGH
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Range: Up to 15mm depending on magnet strength
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 🔌 Pin Assignments & Wiring
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Arduino Uno Complete Pinout
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Function | Pin | Type | Connection | Notes |
| 🔒 Security | Firebase Auth + ESP-NOW encryption ||----------|-----|------|------------|-------|
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Lock #1 Control | D2 | Digital Out | Relay CH1 | LOW = Open |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Lock #2 Control | D3 | Digital Out | Relay CH2 | LOW = Open |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Piezo Buzzer | D4 | PWM Out | Buzzer + | Tone control |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Door Sensor 1 | D5 | Digital In | Reed Switch | 10kΩ pull-up |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Door Sensor 2 | D6 | Digital In | Reed Switch | 10kΩ pull-up |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || SIM800L RX | D8 | Soft Serial RX | SIM800L TX | Voltage divider |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || SIM800L TX | D9 | Soft Serial TX | SIM800L RX | 115200 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ESP32 RX | RX1 (D0) | Serial RX | ESP32 TX | 115200 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ESP32 TX | TX1 (D1) | Serial TX | ESP32 RX | 115200 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### ESP32 Complete Pinout
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Function | Pin | Type | Connection | Notes |
| 🔒 Security | Firebase Auth + ESP-NOW encryption ||----------|-----|------|------------|-------|
| 🔒 Security | Firebase Auth + ESP-NOW encryption || QR Scanner RX | GPIO16 | Serial2 RX | Scanner TX | 9600 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || QR Scanner TX | GPIO17 | Serial2 TX | Scanner RX | 9600 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || LCD SDA | GPIO21 | I2C Data | LCD SDA | 4.7kΩ pull-up |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || LCD SCL | GPIO22 | I2C Clock | LCD SCL | 4.7kΩ pull-up |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Arduino RX | GPIO3 | Serial RX | Arduino TX1 | 115200 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Arduino TX | GPIO1 | Serial TX | Arduino RX1 | 115200 baud |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Power Distribution
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |220V AC (Main) → [S-60-12 Power Supply] → 12V @ 5A
| 🔒 Security | Firebase Auth + ESP-NOW encryption |                        ↓
| 🔒 Security | Firebase Auth + ESP-NOW encryption |        ┌───────────────┼───────────────┐
| 🔒 Security | Firebase Auth + ESP-NOW encryption |        ↓               ↓               ↓
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   [LM2596#1]    [LM2596#2]      [LM2596#3]
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   5V / 2A       5V / 1.5A       5V / 0.5A
| 🔒 Security | Firebase Auth + ESP-NOW encryption |        ↓               ↓               ↓
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   Arduino       SIM800L         LCD+Buzzer
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   + Relay        + GSM            + Logic
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 💾 Firmware Installation Guide
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Arduino Uno Firmware
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**File:** `src/arduino/ParcelBoxUno/ParcelBoxUno.ino` (284 lines)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Setup Steps:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Download Arduino IDE: https://www.arduino.cc/en/software
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Tools → Board → Arduino Uno
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Tools → Port → Select COM port
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Open file and click Upload (Ctrl+U)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. Serial Monitor @ 115200 should show: "[ARDUINO] Parcel Locker Starting..."
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### ESP32 Firmware
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**File:** `src/esp32/ParcelBoxEsp/ParcelBoxEsp.ino` (520+ lines)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Required Modules:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `PINS_CONFIG.h` - Centralized pin definitions
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `FirebaseConfig.h/.cpp` - Firebase credentials
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `WiFiManagerCustom.h/.cpp` - WiFi setup with captive portal
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Add Board Support:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. File → Preferences → Add to "Additional Boards Manager URLs":
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Tools → Boards Manager → Search "ESP32" → Install
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Install Required Libraries:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Tools → Manage Libraries
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Search and install:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **LiquidCrystal_I2C** v1.1.2 (Frank de Brabander)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **Firebase Arduino Client Library** (Mobizt) - Latest version
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **WiFiManager** (tzapu) v2.0.13 or higher
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Critical Setup BEFORE Upload:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Configure Firebase credentials in `FirebaseConfig.cpp` (see Firebase Setup section)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Configure pin assignments if different from defaults (edit `PINS_CONFIG.h`)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Create `.gitignore` with `FirebaseConfig.cpp` to prevent credential leaks
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Upload Steps:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Tools → Board → ESP32 Dev Module
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Tools → Upload Speed → 921600
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Tools → Partition Scheme → Huge APP (for Firebase library)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Upload (Ctrl+U)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. First boot: ESP32 creates "ParcelBox_Setup" WiFi hotspot
| 🔒 Security | Firebase Auth + ESP-NOW encryption |6. Connect phone to hotspot → Browser opens at 192.168.4.1 → Enter WiFi credentials
| 🔒 Security | Firebase Auth + ESP-NOW encryption |7. Serial Monitor @ 115200 should show:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   ```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   [ESP32] WiFi Connected!
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   [ESP32] Device registered in Firebase
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   [ESP32] SYSTEM READY - Waiting for parcel
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   ```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## ⚙️ Configuration & Verification
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Firebase Realtime Database Setup (CRITICAL!)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |The ESP32 uses Firebase Realtime Database for:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Storing parcel delivery orders
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Real-time lock status updates
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Delivery history logging
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Device health monitoring
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 1: Create Firebase Project
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Go to [Firebase Console](https://console.firebase.google.com/)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Click "Create a project"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Enter project name: `parcelbox-yourname`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Accept default settings and create
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 2: Set Up Realtime Database
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. In Dashboard, click "Realtime Database"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Click "Create Database"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. **IMPORTANT:** Select region closest to the locker location (Asia Southeast = optimal)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Start in **Test Mode** (allows reads/writes without authentication)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. Click "Enable"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 3: Get Firebase Credentials
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Click the **Settings gear icon** (top left)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Select "Project Settings"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Go to "Service Accounts" tab
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Click "Generate new private key" → Downloads `google-services.json`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. Open file and copy:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **firebase_database_url:** Extract domain (e.g., `parcelbox-xyz123-default-rtdb.firebaseio.com`)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **api_key:** Under `"api_key": "current_key"`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |   - **project_id:** Under `"project_id"`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 4: Configure ESP32 Firebase Credentials
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Edit `src/esp32/ParcelBoxEsp/FirebaseConfig.cpp`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Replace placeholder values:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const char* ParcelBoxFirebaseConfig::getFirebaseHost() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    return "parcelbox-xyz123-default-rtdb.firebaseio.com";  // ← Your host
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const char* ParcelBoxFirebaseConfig::getDatabaseURL() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    return "https://parcelbox-xyz123-default-rtdb.firebaseio.com";  // ← Your URL
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const char* ParcelBoxFirebaseConfig::getApiKey() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    return "AIza...YOUR_ACTUAL_KEY..."; // ← Your API key
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const char* ParcelBoxFirebaseConfig::getProjectId() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    return "parcelbox-xyz123";  // ← Your project ID
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. **SECURITY:** Add `FirebaseConfig.cpp` to `.gitignore`:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |# .gitignore
| 🔒 Security | Firebase Auth + ESP-NOW encryption |src/esp32/ParcelBoxEsp/FirebaseConfig.cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 5: Set Firebase Security Rules
| 🔒 Security | Firebase Auth + ESP-NOW encryption |In Firebase Console → Realtime Database → Rules tab, paste:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```json
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "rules": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "parcels": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".read": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".write": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      "$parcelId": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |        ".validate": "newData.hasChildren(['qr_code', 'timestamp'])"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    },
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "device_status": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".read": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".write": true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    },
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "locks_status": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".read": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".write": true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    },
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "history": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".read": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".write": true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    },
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "config": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".read": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      ".write": false
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Click "Publish" to apply rules.
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 6: Install Arduino Libraries
| 🔒 Security | Firebase Auth + ESP-NOW encryption |In Arduino IDE → Tools → Manage Libraries, install:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Firebase Arduino Client Library** by Mobizt (latest version)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **WiFiManager** by tzapu (v2.0.13+)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Step 7: Verify Firebase Connection
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Upload to ESP32 and check Serial Monitor:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |[ESP32] Initializing Firebase Realtime Database...
| 🔒 Security | Firebase Auth + ESP-NOW encryption |[ESP32] Device registered in Firebase
| 🔒 Security | Firebase Auth + ESP-NOW encryption |[ESP32] Firebase initialized successfully!
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### WiFi Network
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Must be 2.4GHz:** ESP32 doesn't support 5GHz
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Test with phone hotspot:** Use "iPhone-Network" as test SSID
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Production:** Ensure internet connection for Firebase and API calls
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### I2C LCD Address
| 🔒 Security | Firebase Auth + ESP-NOW encryption |If LCD doesn't display, find correct I2C address with scanner code:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#include <Wire.h>
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void setup() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  Wire.begin(21, 22);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  Serial.begin(115200);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void loop() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  for (int i = 8; i < 120; i++) {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    Wire.beginTransmission(i);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    if (Wire.endTransmission() == 0) {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      Serial.print("Found: 0x");
| 🔒 Security | Firebase Auth + ESP-NOW encryption |      Serial.println(i, HEX);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(5000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Common addresses: 0x27 (default), 0x3F, 0x20, 0x38
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Update `LCD_I2C_ADDRESS` in `PINS_CONFIG.h` with found address
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### WiFi Manager Setup Portal
| 🔒 Security | Firebase Auth + ESP-NOW encryption |ESP32 will create a captive portal during first boot:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Access Point Name:** `ParcelBox_Setup`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Password:** `password123`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Address:** `192.168.4.1` (auto-opens in browser)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Timeout:** 3 minutes (falls back to offline mode)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |To reset WiFi credentials:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |// Call emergencyLockdown() via hardware button or special command
| 🔒 Security | Firebase Auth + ESP-NOW encryption |// Or restart and power on to 2.4GHz WiFi
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### SIM800L Setup (Arduino)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Insert valid SIM card into module
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Verify SIM has SMS support and credit (~PHP 1-5)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Power module 3+ minutes before use
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Verify 3.7-4.2V stable power supply
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Attach external antenna if available
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## ✅ Testing & Validation
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Pre-Power-On Checklist
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] All GND connections common (measure <0.1V difference)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] 12V supply measures 11.5-12.5V
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Each 5V output measures 4.9-5.1V
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] No loose wires or shorts in relay circuit
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Freewheeling diodes installed correctly on solenoids
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- [ ] Reed switches have pull-up resistors
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Component Unit Tests
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Test 1: Relay Click
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void setup() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  pinMode(2, OUTPUT);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  pinMode(3, OUTPUT);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void loop() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  digitalWrite(2, LOW);   // Should CLICK
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(1000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  digitalWrite(2, HIGH);  // Should RELEASE
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(1000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Expected:** Audible relay click, LED blinks
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Test 2: Solenoid Actuation
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Apply 12V directly to solenoid (test externally)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Verify: Audible click, mechanism moves
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Measure current: ~0.8A peak, drops to 0.1-0.2A
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Verify no heat buildup on coil
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Test 3: Buzzer Tones
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void setup() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  pinMode(4, OUTPUT);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void loop() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  tone(4, 1000, 500);   // 1kHz for 500ms
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(1000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  for (int f = 800; f < 2000; f += 100) {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    tone(4, f, 20);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    delay(20);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  for (int i = 0; i < 5; i++) {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    tone(4, 2000, 50);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    delay(100);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(3000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Expected:** Fixed tone, ascending chirp, rapid beeps (3 patterns)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### Test 4: Door Sensors
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```cpp
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void setup() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  pinMode(5, INPUT);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  pinMode(6, INPUT);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  Serial.begin(115200);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |void loop() {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  int d1 = digitalRead(5);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  int d2 = digitalRead(6);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  Serial.println(d1 == HIGH ? "Door1: OPEN" : "Door1: CLOSED");
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  Serial.println(d2 == HIGH ? "Door2: OPEN" : "Door2: CLOSED");
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delay(500);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Procedure:** Move magnet toward reed switch
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Expected:** Clean LOW→HIGH transition, no fluttering
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Full System Workflow Test
| 🔒 Security | Firebase Auth + ESP-NOW encryption |1. Boot both boards - check Serial Monitors for startup messages
| 🔒 Security | Firebase Auth + ESP-NOW encryption |2. Scan QR code "TEST123" - LCD should validate and respond
| 🔒 Security | Firebase Auth + ESP-NOW encryption |3. Locks should click audibly - motors move
| 🔒 Security | Firebase Auth + ESP-NOW encryption |4. Buzzer should beep (success tone)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |5. Close both doors - system detects closure
| 🔒 Security | Firebase Auth + ESP-NOW encryption |6. SMS should be sent within 5 seconds
| 🔒 Security | Firebase Auth + ESP-NOW encryption |7. System resets to "READY" state
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |> ✅ **System ready for production** when ALL tests pass without errors
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 🌐 Backend API Specification
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Required Endpoints
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 1. Validate QR Code
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**POST /api/v1/qr/validate**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```json
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Request:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "qr_code": "PCL-000123",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "locker_id": "LOC-001",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "timestamp": "2026-02-16T10:30:00Z"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Response (Valid):
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "valid": true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "parcel": {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "parcel_id": "PCL-000123",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "receiver_name": "John Doe",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "receiver_phone": "+639171234567",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "locker_number": 1,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "cod_amount": 500
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  },
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "can_open": true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |Response (Invalid):
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "valid": false,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "reason": "PARCEL_NOT_FOUND",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "can_open": false
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 2. Mark Delivery Confirmed
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**POST /api/v1/delivery/confirm**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```json
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "parcel_id": "PCL-000123",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "locker_id": "LOC-001",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "delivery_time": "2026-02-16T10:35:00Z",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "payment_received": true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Triggers:** SMS to receiver, database update, admin notification
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |#### 3. Locker Heartbeat (Health Check)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**POST /api/v1/locker/{locker_id}/heartbeat** (every 60 seconds)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```json
| 🔒 Security | Firebase Auth + ESP-NOW encryption |{
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "locker_id": "LOC-001",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "uptime_seconds": 86400,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "wifi_rssi": -65,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "gsm_signal": 18,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "temperature": 28,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  "firmware_version": "1.0.0"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |}
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Database Schema (Minimal)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```sql
| 🔒 Security | Firebase Auth + ESP-NOW encryption |CREATE TABLE parcels (
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  id INT PRIMARY KEY AUTO_INCREMENT,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  parcel_id VARCHAR(50) UNIQUE NOT NULL,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  qr_code VARCHAR(255) NOT NULL,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  receiver_name VARCHAR(100),
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  receiver_phone VARCHAR(20),
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  locker_id VARCHAR(20),
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  status ENUM('pending', 'ready', 'delivered', 'canceled'),
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  delivered_at TIMESTAMP NULL,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  INDEX idx_qr_code (qr_code),
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  INDEX idx_status (status)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Node.js/Express Example
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```javascript
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const express = require('express');
| 🔒 Security | Firebase Auth + ESP-NOW encryption |const app = express();
| 🔒 Security | Firebase Auth + ESP-NOW encryption |app.use(express.json());
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |app.post('/api/v1/qr/validate', async (req, res) => {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  const { qr_code, locker_id } = req.body;
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  const parcel = await db.query(
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    'SELECT * FROM parcels WHERE qr_code = ?',
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    [qr_code]
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  );
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  if (!parcel || parcel.status === 'delivered') {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    return res.json({ valid: false, can_open: false });
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  res.json({
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    valid: true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    can_open: true,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    parcel: parcel
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  });
| 🔒 Security | Firebase Auth + ESP-NOW encryption |});
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |app.listen(3000);
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## 🔧 Troubleshooting Guide
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || Issue | Solution |
| 🔒 Security | Firebase Auth + ESP-NOW encryption ||-------|----------|
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ Arduino upload fails | Check USB cable (try different cable) \| Verify COM port is selected \| Update CH340 drivers \| Close other serial monitor windows |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ Serial Monitor shows gibberish | Verify baud rate = 115200 (dropdown in Serial Monitor) |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ Relay doesn't click | Verify relay has 5V power (multimeter test) \| Check Arduino pin assignment (D2, D3) \| Test: digitalWrite(2, LOW) directly \| Measure GPIO: should be 0V (LOW), 5V (HIGH) |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ Solenoid doesn't move | Verify 12V reaches solenoid (multimeter) \| Test solenoid directly with external 12V \| Check relay contacts have continuity when activated \| Verify freewheeling diode polarity |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ ESP32 won't connect to WiFi | Verify SSID/Password are correct (case-sensitive) \| Ensure network is 2.4GHz (NOT 5GHz) \| Check signal strength (should be -30 to -65 dBm) \| Try moving locker closer to router |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ LCD display blank | Backlight glows? If no: power issue \| Verify 5V power and pull-up resistors (4.7kΩ) \| Run I2C address scanner to find correct address \| Check for loose SDA/SCL connections |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ QR scanner doesn't read | Scanner LED is on when powered? \| Verify 5V on VCC/GND \| TX/RX pins not reversed? \| Try different QR codes |
| 🔒 Security | Firebase Auth + ESP-NOW encryption || ❌ SMS not sending | SIM card inserted and has credit? \| Test AT command: "AT+CSQ" should show signal ≥10 \| Verify phone number includes country code: +63... \| Power module 3+ seconds before first use |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |## Complete Delivery Workflow (Firebase Integration)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
### Step 1: QR Code Scan (ESP32-CAM via ESP-NOW)

- Rider shows phone screen with QR code to ESP32-CAM
- ESP32-CAM detects and decodes QR using OV2640 camera
- QR data sent wirelessly via ESP-NOW to main ESP32
- ESP-NOW payload: `{qrData: "PKG-12345", timestamp: 123456}`
- ESP-NOW range: ~100m line-of-sight | Latency: <10ms
- System logs event: `history/QR_SCANNED`

**Alternative (Backup): UART Scanner**
- If ESP32-CAM unavailable, rider uses external QR scanner
- Connected to main ESP32 UART1 (GPIO33/26) @ 9600 baud
- Same validation flow applies
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- System logs event to Firebase: `history/QR_SCANNED`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 2: Firebase Validation (Real-Time)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP32 queries Firebase Realtime Database
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Path: `/parcels/{qr_code}`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Firebase returns: Valid parcel data OR null (invalid)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- No need for separate backend API - Firebase handles it
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Response received in <500ms over WiFi
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 3: Lock Opening (If Valid)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP32 sends command: "AT+LOCK1,OPEN\r\n"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Arduino receives and activates GPIO D2
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Relay CH1 energizes → Solenoid #1 clicks open
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- 500ms later: ESP32 sends "AT+LOCK2,OPEN\r\n"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Solenoid #2 (payment box) clicks open
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Arduino responds: "OK\r\n"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP32 triggers buzzer: "AT+BUZZ,SUCCESS\r\n"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- LCD displays: "DOORS OPEN - Place parcel in box"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Firebase updated: `/locks_status/{lock1,lock2}` = "open"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Event logged: `history/VALIDATION_SUCCESS`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 4: Parcel Placement & Payment
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Rider places parcel in open locker
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Rider deposits payment in payment box (if COD)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Rider closes both doors manually
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 5: Door Closure Detection
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Arduino monitors D5 (door 1) and D6 (door 2)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Reed switches detect magnetic closure
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- When both doors LOW: notify ESP32 "EVENT:DOOR1_CLOSED"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Firebase immediately updates: `/locks_status/lock1` = "closed"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 6: Real-Time Firebase Update
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP32 confirms both doors closed
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Sends: "AT+LOCK1,CLOSE" + "AT+LOCK2,CLOSE"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Arduino de-energizes relays (locks engage)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- ESP32 updates parcel status in Firebase:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  - Path: `/parcels/{parcel_id}/status` = "delivered"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  - Timestamp: Current Unix milliseconds
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  - Locker ID: Device ID from Firebase registration
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 7: SMS Notification (Optional - Arduino)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Arduino queries SIM800L for SMS capability
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Can send SMS via SIM800L to receiver
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Message: "Parcel PCL-000123 delivered. Thank you!"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- SMS sends within 5 seconds of door closure
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Step 8: History Logging & System Reset
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Complete delivery event pushed to Firebase:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  ```json
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  {
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "parcel_id": "PCL-000123",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "event": "PARCEL_DELIVERED",
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "timestamp": 1708080600000,
| 🔒 Security | Firebase Auth + ESP-NOW encryption |    "device_id": "PARCELBOX_AABBCCDD"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  }
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  ```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Clears parcel data from memory
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Resets locks to secure position
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- LCD returns to: "READY - Scan parcel QR"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- System ready for next delivery
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Firebase Real-Time Monitoring Dashboard
| 🔒 Security | Firebase Auth + ESP-NOW encryption |During operation, you can view live updates in Firebase Console:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Device Status (Auto-updating every 5 seconds):**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |device_status/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ device_id: "PARCELBOX_A0B1C2D3"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ wifi_connected: true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ firebase_connected: true
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ last_heartbeat: 1708080600000
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Lock Status (Real-time):**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |locks_status/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  ├─ lock1/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  ├─ status: "closed"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  └─ last_update: 1708080595000
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ lock2/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ status: "closed"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     └─ last_update: 1708080595000
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Parcel Records:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |parcels/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ PCL-000123/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ qr_code: "PCL-000123"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ timestamp: 1708080500000
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ status: "delivered"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     └─ locker_compartment: 1
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Delivery History:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |history/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  ├─ -MzK5xY8aB_1/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  ├─ parcel_id: "PCL-000123"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  ├─ event: "QR_SCANNED"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  └─ timestamp: 1708080600100
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  ├─ -MzK5xY8aB_2/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  ├─ parcel_id: "PCL-000123"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  ├─ event: "VALIDATION_SUCCESS"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  │  └─ timestamp: 1708080600200
| 🔒 Security | Firebase Auth + ESP-NOW encryption |  └─ -MzK5xY8aB_3/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ parcel_id: "PCL-000123"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     ├─ event: "PARCEL_DELIVERED"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |     └─ timestamp: 1708080630000
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Security: Unauthorized Access Detection
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- If door opens WITHOUT valid QR scan:
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Reed switch detects door opening (Firebase not updated)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- System checks scan log: valid scan in past 5 minutes?
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- If NO: Flag as UNAUTHORIZED ACCESS
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Event logged to Firebase: `history/UNAUTHORIZED_ACCESS_ATTEMPT`
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- Activate emergency lockdown via `emergencyLockdown()` function
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- System displays: "LOCKDOWN - Contact Admin"
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- All locks engage immediately
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Critical State Variables (Stored in ESP32 RAM)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `device_id` - Generated from MAC address
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `current_parcel_id` - Active delivery QR code
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `lock1_open` / `lock2_open` - Lock status tracking
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `firebase_connected` - Connection state flag
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- `last_scan_time` - Prevents duplicate processing
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |### Firebase Database Structure
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |parcel-box-{projectid}/
| 🔒 Security | Firebase Auth + ESP-NOW encryption |├─ parcels (all pending/delivered parcels)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |├─ device_status (locker health monitoring)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |├─ locks_status (real-time lock state)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |├─ history (complete audit trail)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |└─ config (system configuration)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |```
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |All data is automatically synced across devices and visible in real-time in the Firebase Console.
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |---
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Smart Parcel Locker System v1.0.0**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Status:** Ready for Hardware Assembly & Testing
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Architecture:** Arduino + ESP32 + Firebase Realtime Database
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Cloud Storage:** Unlimited scalability via Google Cloud
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Real-Time Monitoring:** Live Firebase Console dashboard
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Developed By:** Sajed Lopez Mendoza (@qppd)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |**Developer Information:**
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **GitHub:** [github.com/qppd](https://github.com/qppd)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Portfolio:** [sajed-mendoza.onrender.com](https://sajed-mendoza.onrender.com/)
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Email:** quezon.province.pd@gmail.com
| 🔒 Security | Firebase Auth + ESP-NOW encryption |- **Location:** Unisan, Quezon Province, Philippines
| 🔒 Security | Firebase Auth + ESP-NOW encryption |
| 🔒 Security | Firebase Auth + ESP-NOW encryption |All documentation, firmware, and design materials are contained within this README. This is a smart parcel locker system with comprehensive testing, security features, and cloud integration via Firebase Realtime Database.