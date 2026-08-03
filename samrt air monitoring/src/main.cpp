#define BLYNK_TEMPLATE_ID "TMPL3WQpJqayw"
#define BLYNK_TEMPLATE_NAME "smart air monitor"
#define BLYNK_AUTH_TOKEN "q2ng04jbM-917e5f71-9fuzq8j-Xxdz5"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin Definitions ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 34  // Analog input pin

// --- WiFi Credentials ---
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "HOME Wi-Fi";
char pass[] = "MSPP1234";

// --- Create Objects ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 columns, 2 rows

// --- Function to Read Sensors and Send to Blynk + LCD ---
void sendSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // °C
  int airValue = analogRead(MQ135_PIN);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Failed to read from DHT sensor!");
    return;
  }

  // Simple air quality interpretation
  String airStatus;
  if (airValue < 1000) airStatus = "Good";
  else if (airValue < 2000) airStatus = "Moderate";
  else airStatus = "Poor";

  // --- Debug Output ---
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("°C  Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Air: ");
  Serial.print(airValue);
  Serial.print(" -> ");
  Serial.println(airStatus);

  // --- Send Data to Blynk ---
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, airValue);
  Blynk.virtualWrite(V3, airStatus);

  // --- Display on LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C H:");
  lcd.print(humidity, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Air:");
  lcd.print(airStatus);
}

void setup() {
  Serial.begin(115200);
  Serial.println("🌬 Smart Air Monitor Starting...");
Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  dht.begin();
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(300L, sendSensorData);  // Update every 2 seconds
}

void loop() {
  Blynk.run();
  timer.run();
}