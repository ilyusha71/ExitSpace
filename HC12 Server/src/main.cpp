#include <Arduino.h>
/*    Arduino Long Range Wireless Communication using HC-12
                      Example 01
   by Dejan Nedelkovski, www.HowToMechatronics.com
*/
#include <SoftwareSerial.h>
SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin
byte count = 0;

//
#include <DS3231.h>
#include <Wire.h>

DS3231 ds3231;
bool Century, h12, PM;
// Alarm
// byte ADay, AHour, AMinute, ASecond, ABits;
// bool ADy, A12h, Apm;
byte year, month, date, DoW, hour, minute, second;

/****************************************************************************
 * Clock Manage
 * GalaxyClockTimer: 1 tick per second for PC.
 * GroundSynchronizeTimer: HC12 Server transmit synchronize time to other arduinos.
 * TransmitCommandTimer: Transimit server commands that collect from PC.
 ****************************************************************************/
unsigned long GalaxyClockTimer, GroundSynchronizeTimer, TransmitCommandTimer;
int GroundSynchronizeInterval = 3000, TransmitCommandInterval = 500;

void SetTime(byte year, byte month, byte date, byte DoW, byte hour, byte minute, byte second);
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

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
  Serial.begin(9600); // Serial port to computer
  HC12.begin(9600);   // Serial port to HC12
  Wire.begin();
  Serial.println(F("/****************************************************************************"));
  Serial.println(F(" * This is HC12 server"));
  Serial.println(F(" ****************************************************************************/"));
  // SetTime(20, 06, 29, 01, 15, 04, 00);
}

uint8_t rxBuffer, txBuffer;
String rxData = "", txData = "", txDataList[6];
bool txHasData = false;
int txIndex = 0;

String checkingDataList[100];
bool checkingHasData = false, isChecking = false;
int checkingIndex = 0;
int countDevice = 0;

char msgSerial;

void loop()
{
  if (millis() > GalaxyClockTimer)
  {
    GalaxyClockTimer += 1000;

    // 讀取 DS3231 的時間資料
    second = ds3231.getSecond();
    minute = ds3231.getMinute();
    hour = ds3231.getHour(h12, PM);

    // PC Master correction time
    Serial.print(F("Wakaka/DS3231/"));
    hour < 10 ? Serial.print(F("0")) : Serial.print(F(""));
    Serial.print(hour, DEC);
    minute < 10 ? Serial.print(F(":0")) : Serial.print(F(":"));
    Serial.print(minute, DEC);
    second < 10 ? Serial.print(F(":0")) : Serial.print(F(":"));
    Serial.println(second);
  }

  if (millis() > GroundSynchronizeTimer)
  {
    GroundSynchronizeTimer += GroundSynchronizeInterval;

    // 空間Arduino對時
    HC12.print(F("Z/S/"));
    HC12.print(hour, DEC);
    HC12.print(F(":"));
    HC12.print(minute, DEC);
    HC12.print(F(":"));
    HC12.print(second, DEC);
    HC12.println();
  }

  if (millis() > TransmitCommandTimer)
  {
    TransmitCommandTimer += TransmitCommandInterval;
    delay(10);

    for (size_t i = 0; i < SIZE_OF_ARRAY(txDataList); i++)
    {
      if (txDataList[i] != "" && txHasData)
      {
        HC12.println(txDataList[i]);
        txDataList[i] = "";
      }
    }
    txHasData = false;
    txIndex = 0;
  }

  while (HC12.available())
  { // If HC-12 has data
    rxBuffer = HC12.read();
    if (rxBuffer == '\0' || (rxBuffer != 10 && rxBuffer != 13 && rxBuffer < 32) || rxBuffer > 126)
      break;
    rxData += (char)rxBuffer;
    if (rxBuffer == 10) // LF character
    {
      Serial.print(F("Wakaka/"));
      Serial.print(rxData);
      rxData = "";
    }
  }

  while (Serial.available())
  {
    txBuffer = Serial.read();
    if (txBuffer == '\0' || (txBuffer != 10 && txBuffer != 13 && txBuffer < 32) || txBuffer > 126)
      break;
    txData += (char)txBuffer;
    if (txBuffer == 10) // LF character
    {
      if (Split(txData, '/', 0) == "SERVER")
      {
        if (Split(txData, '/', 1) == "GSD")
          GroundSynchronizeInterval = Split(txData, '/', 2).toInt();
        else if (Split(txData, '/', 1) == "TCD")
          TransmitCommandInterval = Split(txData, '/', 2).toInt();
        txData = "";
      }
      else
      {
        if (txIndex == 5) // Max transmission count once
          return;
        txDataList[txIndex] = txData;
        txIndex++;
        txHasData = true;
        txData = "";
      }
    }
  }
}

// String rxHead = getValue(rxData, '/', 0);
// String rxCommand = getValue(rxData, '/', 3);

// if (rxCommand == "Unlock" && txIndex < 5)
// {
//   txData[txIndex] = rxHead + "/" + rxCommand + "/"; // 最後一個斜線用與分離LF
//   txIndex++;
//   txHasData = true;
// }
// 同步到Unity

void SetTime(byte year, byte month, byte date, byte DoW, byte hour, byte minute, byte second)
{
  ds3231.setSecond(second); //Set the second
  ds3231.setMinute(minute); //Set the minute 设置分钟
  ds3231.setHour(hour);     //Set the hour 设置小时
  ds3231.setDoW(DoW);       //Set the day of the week 设置星期几
  ds3231.setDate(date);     //Set the date of the month 设置月份
  ds3231.setMonth(month);   //Set the month of the year 设置一年中的月份
  ds3231.setYear(year);     //Set the year (Last two digits of the year) 设置年份(在今年的最后两位数——比如2013年最后的13)
}
