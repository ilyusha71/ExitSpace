#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(A0,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int kk = analogRead(A0);
  Serial.println (kk);
  // delay(500);
}