/***************************************************
  NodeMCU RFID + Fingerprint + I2C LCD Demo
  - Detect RFID UID over hardware Serial
  - Prompt for fingerprint
  - Show ID 1 or 2 on LCD
***************************************************/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// ---------------- CONFIG ----------------
const String TARGET_RFID = "4C003D46EED9";  // UID to detect

// I2C LCD
LiquidCrystal_I2C lcd(0x27,16,2);

// Fingerprint sensor pins (SoftwareSerial)
#define FINGER_RX_PIN D6  // sensor TX -> D5
#define FINGER_TX_PIN D5  // sensor RX -> D6
SoftwareSerial fingerSerial(FINGER_RX_PIN, FINGER_TX_PIN);
Adafruit_Fingerprint finger(&fingerSerial);

// ---------------- GLOBAL ----------------
String rfidBuffer = "";
unsigned long lastByteTime = 0;
const unsigned long RFID_TIMEOUT = 800;  // ms after last byte to process

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);    // hardware Serial for RFID & debug
  delay(100);

  // LCD init
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("System ready");

  // Fingerprint init
  fingerSerial.begin(57600);
  finger.begin(57600);  // call alone
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor NOT found!");
    lcd.setCursor(0,1);
    lcd.print("Finger err");
    while(1) { delay(1); }  // stop if no sensor
  }

  Serial.println("Waiting for RFID...");
}

// ---------------- LOOP ----------------
void loop() {
  // --- Read RFID from Serial ---
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c >= ' ') rfidBuffer += c;  // keep printable chars
    lastByteTime = millis();
  }

  // Process buffer after timeout
  if (rfidBuffer.length() > 0 && (millis() - lastByteTime) > RFID_TIMEOUT) {
    String packet = rfidBuffer;
    packet.toUpperCase();
    packet.replace(" ",""); packet.replace("\r",""); packet.replace("\n","");
    Serial.println("RFID received: " + packet);

    if (packet.indexOf(TARGET_RFID) >= 0) {
      Serial.println("Target RFID matched!");
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Place finger");
delay(5000);
      int fid = getFingerprintID();
      lcd.clear();

      switch(fid) {
        case 1:
          Serial.println("ID 1 -> RANJITH RAJ");
          lcd.setCursor(0,0); lcd.print("ID 1:");
          lcd.setCursor(0,1); lcd.print("RANJITH RAJ");
          break;
        case 2:
          Serial.println("ID 2 -> SARAN");
          lcd.setCursor(0,0); lcd.print("ID 2:");
          lcd.setCursor(0,1); lcd.print("SARAN");
          break;
          case 3:
          Serial.println("ID 3 -> SENTHIL");
          lcd.setCursor(0,0); lcd.print("ID 3:");
          lcd.setCursor(0,1); lcd.print("SENTHIL");
          break;
          case 4:
          Serial.println("ID 4 -> Dr.velmurugan ");
          lcd.setCursor(0,0); lcd.print("ID 4:");
          lcd.setCursor(0,1); lcd.print("velmurugan");
          break;
        case -1:
          Serial.println("No match or timeout");
          lcd.setCursor(0,0); lcd.print("No match");
          lcd.setCursor(0,1); lcd.print("Try again");
          break;
        default:
          Serial.println("Fingerprint error");
          lcd.setCursor(0,0); lcd.print("Sensor error");
          break;
      }

      delay(2000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ready");
    } else {
      Serial.println("Unauthorized RFID tag detected!");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Unauthorized");
  lcd.setCursor(0,1);
  lcd.print("User (Doctor)");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ready");
    }

    rfidBuffer = "";  // clear buffer
  }

  delay(10);
}

// ---------------- FINGERPRINT HELPER ----------------
int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch(p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Comm error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return -2;
    default:
      Serial.println("Unknown error");
      return -2;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Image->Tz failed");
    return -2;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    Serial.println("No match found");
    return -1;
  }

  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" confidence "); Serial.println(finger.confidence);
  return finger.fingerID;
}
