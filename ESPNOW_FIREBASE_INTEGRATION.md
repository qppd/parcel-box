# ESP-NOW + Firebase Integration Guide

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         PARCEL BOX ECOSYSTEM                             │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────┐              ┌──────────────────────┐            │
│  │   ESP32-CAM      │              │    Main ESP32        │            │
│  │   (Scanner)      │◄──ESP-NOW──►│  (Controller)        │            │
│  │                  │              │                      │            │
│  │ • QR Scanning    │              │ • Lock Control       │            │
│  │ • Image Capture  │              │ • Firebase Streams   │            │
│  │ • Low-level      │              │ • Orchestration      │            │
│  │   processing     │              │ • Device Status      │            │
│  └──────────────────┘              └──────────────────────┘            │
│                                            │                            │
│                                            │ Firebase RTDB             │
│                                            ▼                            │
│                      ┌────────────────────────────────┐               │
│                      │   Firebase Realtime Database   │               │
│                      │                                │               │
│                      │ /device_status/{device_id}    │               │
│                      │ /locks_status/{device_id}     │               │
│                      │ /history/{device_id}/{parcel} │               │
│                      │ /parcels/{parcel_id}          │               │
│                      └────────────────────────────────┘               │
│                                    │                                   │
│                                    │ Bidirectional Streams             │
│                                    ▼                                   │
│                            ┌──────────────┐                           │
│                            │ Flutter App  │                           │
│                            │              │                           │
│                            │ • UI Control │                           │
│                            │ • Status view│                           │
│                            │ • Parcel mgmt│                           │
│                            └──────────────┘                           │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Communication Flow

### QR Scanning Flow (Camera → Main ESP32 → App)

```
1. ESP32-CAM detects QR code
   ↓
2. Sends via ESP-NOW to Main ESP32
   Message: {type: MSG_TYPE_QR_SCAN, qrCode: "..."}
   ↓
3. Main ESP32 receives → ParcelBoxComm::onQrCodeScanned()
   ↓
4. Validates parcel in Firebase
   ↓
5. Logs event: /history/{device_id}/{parcel_id} → "QR_SCANNED_FROM_CAMERA"
   ↓
6. App listens to stream → displays "Parcel detected" ✅
```

### Lock Control Flow (App → Firebase → Main ESP32 → ESP-NOW → Camera)

```
1. User taps "Unlock" in Flutter app
   ↓
2. Writes to Firebase: /device_status/{device_id}/commands/lock1 = "open"
   ↓
3. Main ESP32 stream listener receives command
   ↓
4. ParcelBoxComm::commandLock() triggered
   ↓
5. Sends via ESP-NOW to camera: {type: MSG_TYPE_LOCK_CMD, lock: 1, cmd: 1}
   ↓
6. Camera receives → controls relay/servo
   ↓
7. Lock opens ✅ (or sends feedback)
   ↓
8. Main ESP32 logs: /history/{device_id}/lock_1 → "LOCK_OPENED"
```

### Status Sync Flow (Both → Firebase → App)

```
Main ESP32:
  Every 5 seconds:
  - Reads /device_status/{device_id}/commands (app controls)
  - Updates /device_status/{device_id} (heartbeat, sensors)
  - Updates /locks_status/{device_id} (lock states)
  - Logs to /history/{device_id}/* (events)

ESP32-CAM:
  Every 30 seconds:
  - Sends heartbeat status via ESP-NOW
  - Battery level, signal strength, uptime

App:
  Real-time listeners:
  - onValue(/device_status/{device_id}) → UI status
  - onValue(/locks_status/{device_id}) → lock indicators
  - onValue(/parcels) → parcel list
```

---

## File Structure

```
src/
├── esp32/
│   ├── ParcelBoxEsp/
│   │   ├── ParcelBoxEsp.ino                 (Main sketch)
│   │   ├── FirebaseManager.h/cpp            (Firebase streams)
│   │   ├── EspNowManager.h/cpp              (ESP-NOW communication)
│   │   ├── ParcelBoxComm.h/cpp              (Orchestrator)
│   │   ├── ESPNOW_CONFIG.h                  (Camera MAC addresses)
│   │   └── ... (other existing files)
│   │
│   └── ParcelBoxEspCam/
│       ├── ParcelBoxEspCam.ino              (Camera sketch)
│       ├── EspNowCamera.h/cpp               (Camera ESP-NOW)
│       └── ... (image processing, QR scanning)
│
└── app/
    └── lib/
        ├── services/
        │   ├── database_service.dart        (Read/write Firebase)
        │   └── auth_service.dart
        └── ... (UI screens)
```

---

## Integration Checklist

### ESP32-CAM Side

- [ ] Include `EspNowCamera.h` in `ParcelBoxEspCam.ino`
- [ ] Initialize in `setup()`: `espNowCam.begin("ESP32CAM_001", mainEspMac)`
- [ ] When QR detected: `espNowCam.sendQrCode(qrCodeString)`
- [ ] In `loop()`: check `espNowCam.hasCommand()` for lock commands
- [ ] Implement `onLockCommand()` to control relays/servos

Example:
```cpp
#include "EspNowCamera.h"

EspNowCamera espNowCam;
uint8_t mainEspMac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};  // Set actual MAC

void setup() {
  espNowCam.begin("ESP32CAM_001", mainEspMac);
}

void loop() {
  // When QR code detected
  if (qrDetected) {
    espNowCam.sendQrCode(detectedQR);
  }
  
  // Check for commands
  if (espNowCam.hasCommand()) {
    espNowCam.processCommand();
  }
}

void EspNowCamera::onLockCommand(uint8_t lockNum, uint8_t command, uint16_t duration) {
  if (command == 1) {  // UNLOCK
    openRelay(lockNum);
    delay(duration ? duration : 3000);  // Default 3 seconds
    closeRelay(lockNum);
  } else {
    closeRelay(lockNum);
  }
}
```

### Main ESP32 Side

- [x] Include `ParcelBoxComm.h`
- [x] Create global: `ParcelBoxComm comms;`
- [x] In `setup()`: `comms.begin(deviceId, cameraMac);`
- [x] In `loop()`: `comms.handle();`
- [x] When QR received: call `comms.onQrCodeScanned(qr)`
- [ ] When lock needed: call `comms.commandLock(lockNum, unlock, duration)`

### Flutter App Side

1. **Listen to device status**:
```dart
FirebaseDatabase.instance
    .ref('/device_status')
    .child(deviceId)
    .onValue
    .listen((event) {
  setState(() {
    deviceStatus = event.snapshot.value;
  });
});
```

2. **Listen to lock status**:
```dart
FirebaseDatabase.instance
    .ref('/locks_status')
    .child(deviceId)
    .onValue
    .listen((event) {
  setState(() {
    locksStatus = event.snapshot.value;
  });
});
```

3. **Send lock commands**:
```dart
await FirebaseDatabase.instance
    .ref('/device_status')
    .child(deviceId)
    .child('commands')
    .child('lock1')
    .set('open');
```

4. **Listen to parcel history**:
```dart
FirebaseDatabase.instance
    .ref('/history')
    .child(deviceId)
    .onValue
    .listen((event) {
  // Update parcel delivery history
});
```

---

## Message Protocol

### ESP-NOW Message Structure (296 bytes)

```cpp
struct EspNowMessage {
  uint8_t type;           // 0=QR, 1=LOCK, 2=STATUS, 3=CONFIG, 4=ACK
  char deviceId[32];      // Source device identifier
  uint32_t timestamp;     // Milliseconds since boot
  uint8_t retries;        // Retry counter for reliability
  
  union payload {
    char qrCode[64];                // QR_SCAN message
    {lock, command, duration}       // LOCK_CMD message
    {battery, signal, uptime}       // STATUS message
    char configData[128];           // CONFIG message
  }
}
```

### Message Examples

**QR Scan** (Camera → Main ESP32):
```
{
  type: 0,
  deviceId: "ESP32CAM_001",
  timestamp: 1234567890,
  payload.qrCode: "PARCEL_20260509_12345"
}
```

**Lock Command** (Main ESP32 → Camera):
```
{
  type: 1,
  deviceId: "ESP32MAIN_001",
  timestamp: 1234567891,
  payload.lockCmd: {
    lock: 1,
    command: 1,  // 0=close, 1=open
    duration: 3000  // milliseconds
  }
}
```

**Status Update** (Camera → Main ESP32):
```
{
  type: 2,
  deviceId: "ESP32CAM_001",
  timestamp: 1234567892,
  payload.status: {
    battery: 85,
    signalStrength: -65,
    uptime: 3600
  }
}
```

---

## Firebase Paths Reference

All paths use the standardized structure:

```
/device_status/{device_id}/
  ├─ wifi_connected: boolean
  ├─ firebase_connected: boolean
  ├─ temperature: float
  ├─ humidity: int
  ├─ last_heartbeat: int64
  └─ /commands/
      ├─ lock1: "open"|"closed"
      ├─ lock2: "open"|"closed"
      ├─ door1: "open"|"closed"
      └─ emergency_unlock: boolean

/locks_status/{device_id}/
  ├─ lock1: "open"|"closed"
  ├─ lock2: "open"|"closed"
  ├─ door1: "open"|"closed"
  ├─ door2: "open"|"closed"
  └─ timestamp: int64

/history/{device_id}/{parcel_id}/
  └─ [event1, event2, ...] with timestamps

/parcels/{parcel_id}/
  ├─ status: "pending"|"delivered"|"failed"
  ├─ location: "box_1"|"box_2"
  ├─ timestamp: int64
  └─ ... (app-managed fields)
```

---

## Testing & Debugging

### 1. Check ESP-NOW Peers
```cpp
comms.getEspNowManager()->printPeerInfo();
```

Output:
```
[ESP-NOW] Peer Information:
  Messages Sent: 42
  Messages Received: 38
  Messages Failed: 2
  Success Rate: 95.2%
```

### 2. Monitor Communication
```cpp
Serial.println("Camera connected: " + String(comms.isCameraConnected()));
Serial.println("Firebase ready: " + String(comms.getFirebaseManager()->isReady()));
```

### 3. Test QR Scanning
Simulate in Arduino IDE:
```cpp
comms.onQrCodeScanned("TEST_QR_12345");
```

### 4. Firebase Console
Watch real-time updates:
- Realtime Database → `/device_status` → see heartbeats
- Realtime Database → `/history` → see events
- Realtime Database → `/locks_status` → see lock states

---

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| QR not received | Wrong camera MAC | Update in `ESPNOW_CONFIG.h` |
| Lock not opening | No ESP-NOW peer | Add camera MAC via `comms.addCameraPeer()` |
| Firebase disconnects | WiFi issue | Check `checkWiFiConnection()` |
| High message loss | Poor signal | Reduce message size or add retries |
| App doesn't see updates | No Firebase stream | Verify `onValue` listener in app |

---

## Performance Metrics

```
ESP-NOW:
  - Range: ~250 meters (line of sight)
  - Latency: < 50ms
  - Payload: up to 250 bytes
  - Success rate: 95-99%

Firebase Streams:
  - Latency: 100-500ms (network dependent)
  - Real-time: Yes (push-based)
  - Cost: Very low for reads

Combined:
  - Device-to-device: < 100ms (ESP-NOW)
  - Device-to-app: 100-500ms (Firebase)
  - Total round-trip: ~600ms
```

---

## Next Steps

1. **Get actual ESP32-CAM MAC address**:
   ```cpp
   Serial.println(WiFi.macAddress());  // Run on camera
   ```

2. **Update `ESPNOW_CONFIG.h`** with real MAC

3. **Test pairing** in both devices

4. **Implement lock control** callbacks in camera code

5. **Deploy** to both devices

6. **Monitor** via Serial and Firebase Console
