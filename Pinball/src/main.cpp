#include <Arduino.h>
#define AN_V1WC1 A0
#define AN_H3WU7 A1
#define AN_V3WU7 A2
#define V1WC1 7
#define H3WU7 6
#define V3WU7 5

int rV1WC1, countV1WC1, rH3WU7, countH3WU7, rV3WU7, countV3WU7;

void setup()
{
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(AN_V1WC1, INPUT);
  pinMode(AN_H3WU7, INPUT);
  pinMode(AN_V3WU7, INPUT);
  pinMode(V1WC1, OUTPUT);
  pinMode(H3WU7, OUTPUT);
  pinMode(V3WU7, OUTPUT);
  digitalWrite(V1WC1, HIGH);
  digitalWrite(H3WU7, HIGH);
  digitalWrite(V3WU7, HIGH);
}

void loop()
{
  if (analogRead(AN_V1WC1) < 700)
  {
    rV1WC1++;
    if (rV1WC1 > 3)
    {
      Serial.println("V1WC1");
      rV1WC1 = 0;
      digitalWrite(V1WC1, LOW);
      delay(3000);
      digitalWrite(V1WC1, HIGH);
    }
  }
  if (analogRead(AN_H3WU7) < 700)
  {
    rH3WU7++;
    if (rH3WU7 > 3)
    {
      Serial.println("H3WU7");
      rH3WU7 = 0;
      digitalWrite(H3WU7, LOW);
      delay(3000);
      digitalWrite(H3WU7, HIGH);
    }
  }
  if (analogRead(AN_V3WU7) < 700)
  {
    rV3WU7++;
    if (rV3WU7 > 3)
    {
      Serial.println("V3WU7");
      rV3WU7 = 0;
      digitalWrite(V3WU7, LOW);
      delay(3000);
      digitalWrite(V3WU7, HIGH);
    }
  }
}