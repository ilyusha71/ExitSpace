#include <Arduino.h>
#define ANALOG_V1WC1 A0
#define ANALOG_H3WU7 A1
#define ANALOG_V3WU7 A2
#define RELAY_V1WC1 7
#define RELAY_H3WU7 6
#define RELAY_V3WU7 5

int rank[3];            // 類比累加
float avgVolt[3];       // 類比平均
int counter;            // 循環次數
int count[3];           // 觸發次數
unsigned long timer[3]; // 下次觸發時間

/****************************************************************************
 * 可調控
 ****************************************************************************/
int declineFactor[3]; // 觸發底限
int interval[3];      // 觸發最小間隔

String DEVICE_NAME = "Pinball";

/****************************************************************************
 * HC-12
 ****************************************************************************/
#include <SoftwareSerial.h>
#define SETUP 4
SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin
bool rxHasData = false;
uint8_t rxBuffer;
String rxData = "";
int cbDelay = 100;
unsigned long cbClock;

/****************************************************************************
 * Split Method
 ****************************************************************************/
String Split(String data, char separator, int index);
String Split(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup()
{
  Serial.begin(9600);
  HC12.begin(9600); // Serial port to HC12
  pinMode(ANALOG_V1WC1, INPUT);
  pinMode(ANALOG_H3WU7, INPUT);
  pinMode(ANALOG_V3WU7, INPUT);
  pinMode(RELAY_V1WC1, OUTPUT);
  pinMode(RELAY_H3WU7, OUTPUT);
  pinMode(RELAY_V3WU7, OUTPUT);
  digitalWrite(RELAY_V1WC1, HIGH);
  digitalWrite(RELAY_H3WU7, HIGH);
  digitalWrite(RELAY_V3WU7, HIGH);

  for (size_t i = 0; i < 3; i++)
  {
    declineFactor[i] = 50;
    interval[i] = 500;
  }
}

void loop()
{
  while (HC12.available())
  { // If HC-12 has data
    rxBuffer = HC12.read();
    if (rxBuffer == '\0' || (rxBuffer != 10 && rxBuffer != 13 && rxBuffer < 32) || rxBuffer > 126)
      break;
    rxData += (char)rxBuffer;
    if (rxBuffer == 10) // LF character
    {
      String rxHead = Split(rxData, '/', 1);
      if (rxHead == "S") // Server Main Clock
      {
        Serial.print(rxData); // 記錄時間
        cbClock = millis() + cbDelay;
      }
      else if (rxHead == DEVICE_NAME)
      {
        Serial.print(rxData);
        String rxCommand = Split(rxData, '/', 3);
        if (rxCommand == "Checking")
        {
          rxHasData = true;
          cbDelay = Split(rxData, '/', 4).toInt();
        }
        else if (rxCommand == "SetDecline")
          declineFactor[Split(rxData, '/', 2).toInt()] = Split(rxData, '/', 4).toInt();
        else if (rxCommand == "SetInterval")
          interval[Split(rxData, '/', 2).toInt()] = Split(rxData, '/', 4).toInt();
      }
      rxData = "";
    }
  }

  if (millis() > cbClock && rxHasData)
  {
    rxHasData = false;
    HC12.print(F("Z/"));
    HC12.print(DEVICE_NAME);
    HC12.print(F("/avg/"));
    HC12.print(avgVolt[0]);
    HC12.print(F("/"));
    HC12.print(avgVolt[1]);
    HC12.print(F("/"));
    HC12.print(avgVolt[2]);
    HC12.println(F("/"));
    // HC12.print("Z/Callback/");
    // HC12.println(DEVICE_NAME);
  }

  /* HC-12 END */
  if (analogRead(ANALOG_V1WC1) < avgVolt[0] - declineFactor[0] && millis() > timer[0])
  {
    timer[0] += interval[0];
    count[0]++;
    Serial.print(F("V1WC1 count: "));
    Serial.println(count[0]);

    HC12.print(F("Z/"));
    HC12.print(DEVICE_NAME);
    HC12.print(F("/0/"));
    HC12.print(count[0]);
    HC12.print(F("/"));
    HC12.print(analogRead(ANALOG_V1WC1));
    HC12.println(F("/"));
    delay(500);
    if (count[0] > 3)
    {
      count[0] = 0;
      digitalWrite(RELAY_V1WC1, LOW);
      delay(3000);
      digitalWrite(RELAY_V1WC1, HIGH);
    }
  }

  if (analogRead(ANALOG_H3WU7) < avgVolt[1] - declineFactor[1] && millis() > timer[1])
  {
    timer[1] += interval[1];
    count[1]++;
    Serial.print(F("H3WU7 count: "));
    Serial.println(count[1]);

    HC12.print(F("Z/"));
    HC12.print(DEVICE_NAME);
    HC12.print(F("/1/"));
    HC12.print(count[1]);
    HC12.print(F("/"));
    HC12.print(analogRead(ANALOG_H3WU7));
    HC12.println(F("/"));
    delay(500);
    if (count[1] > 3)
    {
      count[1] = 0;
      digitalWrite(RELAY_H3WU7, LOW);
      delay(3000);
      digitalWrite(RELAY_H3WU7, HIGH);
    }
  }

  if (analogRead(ANALOG_V3WU7) < avgVolt[2] - declineFactor[2] && millis() > timer[2])
  {
    timer[2] += interval[2];
    count[2]++;
    Serial.print(F("V3WU7 count: "));
    Serial.println(count[2]);

    HC12.print(F("Z/"));
    HC12.print(DEVICE_NAME);
    HC12.print(F("/2/"));
    HC12.print(count[2]);
    HC12.print(F("/"));
    HC12.print(analogRead(ANALOG_V3WU7));
    HC12.println(F("/"));
    delay(500);
    if (count[2] > 3)
    {
      count[2] = 0;
      digitalWrite(RELAY_V3WU7, LOW);
      delay(3000);
      digitalWrite(RELAY_V3WU7, HIGH);
    }
  }

  if (counter < 10)
  {
    // Serial.println(F("GOGOGO"));

    rank[0] += analogRead(ANALOG_V1WC1);
    rank[1] += analogRead(ANALOG_H3WU7);
    rank[2] += analogRead(ANALOG_V3WU7);
    counter++;
    if (counter >= 10)
    {
      for (size_t i = 0; i < 3; i++)
      {
        avgVolt[i] = rank[i] * 0.1f;
        rank[i] = 0;
      }
      counter = 0;
    }
  }
  delay(100);
}