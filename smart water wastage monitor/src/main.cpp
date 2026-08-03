// ================= 0. BLYNK SETUP =================
#define BLYNK_TEMPLATE_ID "TMPL35tXyvsO6"
#define BLYNK_TEMPLATE_NAME "water wastage monitor"
#define BLYNK_AUTH_TOKEN "PObvFsTwJhuGHJwY1x-SKDJIV3vOrQax"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <NewPing.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= 1. CREDENTIALS & PINS =================
// --- Networking ---
#define WIFI_SSID "HOME Wi-Fi"
#define WIFI_PASSWORD "MSPP9347"

// --- Hardware Pins ---
#define LCD_ADDR 0x27  // I2C LCD Address

// Sensors
#define FLOW_SENSOR_PIN 34   // Flow sensor on GPIO 34
#define TRIG_STORAGE    14
#define ECHO_STORAGE    18
#define TRIG_WASTAGE    19
#define ECHO_WASTAGE    23

// LEDs (only red ones used)
const int sRed = 27;     // Storage Red LED
const int wRed = 33;     // Wastage Red LED

// Smart Tap
#define IR_SENSOR_PIN 32       // IR sensor digital output
#define RELAY_PIN     25       // Relay module control pin


// ================= 2. OBJECTS & GLOBAL VARIABLES =================
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

NewPing sonarStorage(TRIG_STORAGE, ECHO_STORAGE, 200);
NewPing sonarWastage(TRIG_WASTAGE, ECHO_WASTAGE, 200);

// --- Storage Tank ---
const float tankHeight = 17.8;       // cm
const float tankVolume = 4.28;       // liters
const float litersPerCM = tankVolume / tankHeight;

// --- Wastage Tank ---
const float wastageHeight = 30.0;    // cm
const float wastageVolume = 5.31;    // liters (cylindrical tank calc)
const float wastageLitersPerCM = wastageVolume / wastageHeight;

volatile long pulseCount = 0;
float flowRate = 0.0;        // in mL/sec
float storageLiters = 0.0;
float wastageLiters = 0.0;
unsigned long flowCalcTime = 0;
const float flowCalibration = 7.5;  // adjust experimentally for ZJ-S201

int loopCounter = 0;
// Smart Tap variables
bool tapOn = false;
unsigned long irDetectTime = 0;


// ================= 3. INTERRUPT SERVICE ROUTINE =================
void IRAM_ATTR flowISR() {
  pulseCount++;
}

// ================= 4. SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(sRed, OUTPUT);
  pinMode(wRed, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowISR, FALLING);
   pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // tap OFF initially


  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
    lcd.clear();
    lcd.print("WiFi Connected!");
  } else {
    lcd.clear();
    lcd.print("WiFi Error!");
    Serial.println("WiFi not connected, continuing offline...");
  }
}

// ================= TAP HANDLING FUNCTION =================
void handleTap() {
  // If IR sensor detects hand
  if (digitalRead(IR_SENSOR_PIN) == HIGH) {
    if (irDetectTime == 0) irDetectTime = millis(); // start timing
    if (!tapOn && millis() - irDetectTime >= 700) {
      tapOn = true;
      digitalWrite(RELAY_PIN, HIGH); // turn ON tap
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Tap");lcd.print("ON");
      Serial.println("Tap switched ON");
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(V4, 1); // upload tap status
      }
    }
  } else {
    digitalWrite(RELAY_PIN, LOW);
    tapOn = false;
    irDetectTime = 0; // reset timer
    if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(V4, 0); // one update to Blynk when OFF
      }
  }
}


// ================= 5. MAIN LOOP =================
void loop() {
  // --- WiFi/Blynk handling ---
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }
  // --- 1. FLOW RATE ---
  unsigned long currentTime = millis();
  if (currentTime - flowCalcTime >= 1000) {
    float timeSec = (currentTime - flowCalcTime) / 1000.0;
    flowRate = (pulseCount * 1000) / (flowCalibration * timeSec);
    pulseCount = 0;
    flowCalcTime = currentTime;
  }
  handleTap();

  // --- 2. ULTRASONIC READINGS ---
  int dStorage = sonarStorage.ping_cm();
  int dWastage = sonarWastage.ping_cm();

  if (dStorage == 0) dStorage = 200;
  if (dWastage == 0) dWastage = 200;

  storageLiters = tankHeight - dStorage;
  wastageLiters = wastageHeight - dWastage;

  if (storageLiters < 0) storageLiters = 0;
  if (wastageLiters < 0) wastageLiters = 0;

  storageLiters *= litersPerCM;
  wastageLiters *= wastageLitersPerCM;


  // --- 3. LED LOGIC ---
  digitalWrite(wRed, wastageLiters > 2 ? HIGH : LOW);
  digitalWrite(sRed, storageLiters <= 1 ? HIGH : LOW);

  // --- 4. LCD PAGE 1 ---
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Strg: "); lcd.print(storageLiters, 1); lcd.print("L");
  lcd.setCursor(0, 1); lcd.print("Wstg: "); lcd.print(wastageLiters, 1); lcd.print("L");
  delay(1400);

  // --- 5. LCD PAGE 2 ---
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Flow Rate:");
  lcd.setCursor(0, 1); lcd.print(flowRate, 2); lcd.print(" mL/sec");
  delay(1400);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Tap");lcd.print(tapOn);
  handleTap();
  delay(1200);
  


  // --- 6. BLYNK UPLOAD ---
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.virtualWrite(V1, storageLiters);
    Blynk.virtualWrite(V2, wastageLiters);
    Blynk.virtualWrite(V3, flowRate);
  }



  Serial.print("S: "); Serial.print(storageLiters);
  Serial.print(" L | W: "); Serial.print(wastageLiters);
  Serial.print(" L | Flow: "); Serial.println(flowRate);

  // --- WiFi retry logic ---
  loopCounter++;
  if (loopCounter >= 10) {
    loopCounter = 0;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Retrying WiFi...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      unsigned long retryStart = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - retryStart < 5000) {
        delay(500);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED) {
        lcd.clear();
        lcd.print("WiFi Connected!");
        Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
      } else {
        Serial.println("WiFi still not connected.");
      }
    }
  }
} 

