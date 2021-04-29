#include <Arduino.h>
String DEVICE_NAME = "BadgeGate";
#define BADGE 7
/****************************************************************************
 * HC-12
 *  HC12_SET
 *    HIGH ==> Work
 *    LOW  ==> AT Command
 *  SoftwareSerial(rx,tx)
 *    Arduino rx <===> HC12 tx
 *    Arduino tx <===> HC12 rx
 ****************************************************************************/
#include <SoftwareSerial.h>
#define HC12_TX 2
#define HC12_RX 3
#define HC12_SET 4
SoftwareSerial HC12(HC12_TX, HC12_RX);
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

/****************************************************************************
 * Main
 ****************************************************************************/
void setup()
{
  Serial.begin(9600);
  HC12.begin(9600);
  pinMode(HC12_SET, OUTPUT);
  digitalWrite(HC12_SET, HIGH);
  pinMode(BADGE, OUTPUT);
  digitalWrite(BADGE, HIGH);
  Serial.println(F("BadgeGate"));
}

void loop()
{
  while (HC12.available())
  { // If HC-12 has data
    rxBuffer = HC12.read();
    // Ignor control character except LF(10) and CR(13)
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
        String rxCommand = Split(rxData, '/', 2);
        if (rxCommand == "Pass")
          digitalWrite(BADGE, LOW);
        else if (rxCommand == "Checking")
        {
          rxHasData = true;
          cbDelay = Split(rxData, '/', 3).toInt();
        }
      }
      rxData = "";
    }
  }

  if (millis() > cbClock && rxHasData)
  {
    rxHasData = false;
    HC12.print("Z/Callback/");
    HC12.println(DEVICE_NAME);
  }
}