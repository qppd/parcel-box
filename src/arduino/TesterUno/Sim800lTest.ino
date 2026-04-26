/*
  ESP32 (38-pin) + SIM800L AT Command Tester
  TX -> GPIO17
  RX -> GPIO16
*/

#define SIM800_TX 17
#define SIM800_RX 16

HardwareSerial sim800(2);   // Use UART2

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Initializing SIM800L...");

  // Begin SIM800L serial communication
  sim800.begin(9600, SERIAL_8N1, SIM800_RX, SIM800_TX);
  delay(3000);

  Serial.println("Type AT commands below:");
}

void loop() {
  // If something comes from SIM800L, print to Serial Monitor
  while (sim800.available()) {
    Serial.write(sim800.read());
  }

  // If something is typed in Serial Monitor, send to SIM800L
  while (Serial.available()) {
    sim800.write(Serial.read());
  }
}
