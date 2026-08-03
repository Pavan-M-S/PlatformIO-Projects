// Pin assignments
#include <Arduino.h>
const int sGreen  = 32;
const int sYellow = 4;
const int sRed    = 27;

const int wGreen  = 26;
const int wYellow = 25;
const int wRed    = 33;

void setup() {
  // Configure pins as outputs
  pinMode(sGreen, OUTPUT);
  pinMode(sYellow, OUTPUT);
  pinMode(sRed, OUTPUT);

  pinMode(wGreen, OUTPUT);
  pinMode(wYellow, OUTPUT);
  pinMode(wRed, OUTPUT);

  // Set all pins HIGH
  digitalWrite(sGreen, HIGH);
  digitalWrite(sYellow, HIGH);
  digitalWrite(sRed, HIGH);

  digitalWrite(wGreen, HIGH);
  digitalWrite(wYellow, HIGH);
  digitalWrite(wRed, HIGH);
}

void loop() {
  // Nothing needed here unless you want to toggle pins later
}