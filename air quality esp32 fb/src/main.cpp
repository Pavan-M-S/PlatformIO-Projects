#include <Arduino.h>

// Most standard ESP32 Dev boards have a built-in blue LED on GPIO 2
const int LED_PIN = 2; 

void setup() {
  // Start the serial communication at a baud rate of 115200
  Serial.begin(115200);
  
  // Configure the built-in LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  delay(4000);
  // Print a startup message to the Serial Monitor
  Serial.println("ESP32 Boot Successful! Test code running...");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);   // Turn the LED on
  Serial.println("LED is ON");
  delay(1000);                   // Wait for 1 second (1000 milliseconds)
  
  digitalWrite(LED_PIN, LOW);    // Turn the LED off
  Serial.println("LED is OFF");
  delay(1000);                   // Wait for 1 second
}