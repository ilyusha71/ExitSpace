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

DS3231 clock;
bool Century, h12, PM;
// Alarm
// byte ADay, AHour, AMinute, ASecond, ABits;
// bool ADy, A12h, Apm;
byte year, month, date, DoW, hour, minute, second;
unsigned long galaxyTimer, spaceTimer, commandTimer;
void SetTime(byte year, byte month, byte date, byte DoW, byte hour, byte minute, byte second);
String getValue(String data, char separator, int index);
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

void setup()
{
  Serial.begin(9600); // Serial port to computer
  HC12.begin(9600);   // Serial port to HC12
  Wire.begin();
  Serial.println(F("/****************************************************************************"));
  Serial.println(F(" * This is HC12 server"));
  Serial.println(F(" ****************************************************************************/"));
  // SetTime(20, 03, 26, 04, 10, 49, 00);
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
  if (millis() > galaxyTimer && !isChecking)
  {
    galaxyTimer += 1000;
    // 讀取 DS3231 的時間資料
    second = clock.getSecond();
    minute = clock.getMinute();
    hour = clock.getHour(h12, PM);
    // 電腦Server對時
    Serial.print(F("Wakaka/Clock/"));
    hour < 10 ? Serial.print(F("0")) : Serial.print(F(""));
    Serial.print(hour, DEC);
    minute < 10 ? Serial.print(F(":0")) : Serial.print(F(":"));
    Serial.print(minute, DEC);
    second < 10 ? Serial.print(F(":0")) : Serial.print(F(":"));
    Serial.println(second);
  }
  if (millis() > spaceTimer)
  {
    spaceTimer += 3000;
    // 空間Arduino對時
    HC12.print(F("Z/S/"));
    HC12.print(hour, DEC);
    HC12.print(F(":"));
    HC12.print(minute, DEC);
    HC12.print(F(":"));
    HC12.print(second, DEC);
    HC12.println();
  }
  if (millis() > commandTimer)
  {
    commandTimer += 2000;
    delay(10);

    // if (checkingHasData)
    // {
    //   for (size_t i = 0; i < SIZE_OF_ARRAY(checkingDataList); i++)
    //   {
    //     if (checkingDataList[i] != "" && checkingHasData)
    //     {
    //       delay(100);
    //       // Serial.print(F("Wakaka/Server/"));
    //       // Serial.print(checkingDataList[i]);
    //       HC12.println(checkingDataList[i]);
    //       checkingDataList[i] = "";
    //     }
    //   }
    //   checkingHasData = false;
    //   checkingIndex = 0;
    // }
    // else
    {

      // 訊息驗證 Callback
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
  { // If Serial monitor has data
    txBuffer = Serial.read();
    if (txBuffer == '\0' || txBuffer == 255)
      break;
    txData += (char)txBuffer;
    if (txBuffer == 10) // LF character
    {
      // String checkHead = getValue(txData, '/', 1);
      // if (checkHead == "CheckingStart")
      // {
      //   isChecking = true;
      //   countDevice = getValue(txData, '/', 2).toInt();
      //   Serial.print(F("Wakaka/Server/000/000/Checking Start/"));
      //   Serial.println(countDevice);
      //   checkingIndex = 0;
      // }
      // else if (checkHead == "CheckingEnd")
      // {
      //   isChecking = false;
      //   countDevice = getValue(txData, '/', 2).toInt();
      //   Serial.print(F("Wakaka/Server/000/000/Checking End/"));
      //   Serial.print(checkingIndex);
      //   Serial.print(F("/"));
      //   Serial.println(countDevice);
      //   checkingIndex = 0;
      // }
      // else if (checkHead == "Checking End")
      // {
      //   isChecking = false;
      //   if (checkingIndex == countDevice)
      //   {
      //     // checkingHasData = true;
      //     Serial.print(F("Wakaka/Server/dd/Server/Checking End/"));
      //   }
      //   else
      //     Serial.print(F("Wakaka/Server/dd/Server/Checking Fail/"));
      //   Serial.println(checkingIndex);
      //   checkingIndex = 0;
      // }
      // else
      {
        // if (isChecking)
        // {
        //   Serial.print(F("Wakaka/Server/"));
        //   Serial.print(checkingIndex);
        //   Serial.print(F("/"));
        //   Serial.print(txData);
        //   checkingDataList[checkingIndex] = txData;
        //   checkingIndex++;
        // }
        // else
        {
          // Serial.print(F("Wakaka/Server/"));
          // Serial.print(checkingIndex);
          // Serial.print(F("/NO/"));
          // Serial.print(txData);
          // checkingIndex++;
          if (txIndex == 5)
            return;
          txDataList[txIndex] = txData;
          txIndex++;
          txHasData = true;
        }
      }
      txData = "";
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
  clock.setSecond(second); //Set the second
  clock.setMinute(minute); //Set the minute 设置分钟
  clock.setHour(hour);     //Set the hour 设置小时
  clock.setDoW(DoW);       //Set the day of the week 设置星期几
  clock.setDate(date);     //Set the date of the month 设置月份
  clock.setMonth(month);   //Set the month of the year 设置一年中的月份
  clock.setYear(year);     //Set the year (Last two digits of the year) 设置年份(在今年的最后两位数——比如2013年最后的13)
}

String getValue(String data, char separator, int index)
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