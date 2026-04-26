#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int relay1Pin = 10;
const int relay2Pin = 11;
const int buzzerPin = 12;
const int reed1Pin = 8;
const int reed2Pin = 9;

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(reed1Pin, INPUT_PULLUP);
  pinMode(reed2Pin, INPUT_PULLUP);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {
  // Buzzer ON with tone
  // tone(buzzerPin, 1000); // 1kHz tone
  // delay(1000); // 1 second ON

  // // Buzzer OFF
  // noTone(buzzerPin);
  // delay(1000); // 1 second OFF

  // Reed switch test
  int reed1State = digitalRead(reed1Pin);
  int reed2State = digitalRead(reed2Pin);
  Serial.print("Reed 1: ");
  Serial.print(reed1State == LOW ? "CLOSED" : "OPEN");
  Serial.print(" | Reed 2: ");
  Serial.println(reed2State == LOW ? "CLOSED" : "OPEN");

  lcd.setCursor(0, 0);
  lcd.print("Reed1: ");
  lcd.print(reed1State == LOW ? "CLOSED " : "OPEN   ");
  lcd.setCursor(0, 1);
  lcd.print("Reed2: ");
  lcd.print(reed2State == LOW ? "CLOSED " : "OPEN   ");

  delay(500);
}
