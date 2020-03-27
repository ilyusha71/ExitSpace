#include <Arduino.h>
// master_sender.ino
// Refer to the "slave_receiver" example for use with this
#define I2C_BUFFER_SIZE 32
#include <DS3231.h>
// 0x57, 0x68 為DS3231預設使用I2C地址
DS3231 clock;
bool Century = false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;
byte year, month, date, DoW, hour, minute, second;

const int SLAVE_ADDRESS = 15;
char incomingByte = 0;
byte messages[32];

uint8_t i2cBuffer[I2C_BUFFER_SIZE];
uint8_t i2cBufferCnt = 0;
uint8_t DATA_SIZE = 32;

/****************************************************************************
 * 重設關卡時限
 * 999:Stage/0/900
 * 開門
 * 36:Unlock
 * 重啟
 * 74:Reset
 ****************************************************************************/
// 重設關卡時限
// boolean
bool resetADN = false;
bool resetStageLimit = false;
bool check = true;
bool unlock = true;
int indexStage = 0;
long timeLimit = 86400;

int targetAddress = 58;

void ModTime(byte year, byte month, byte date, byte DoW, byte hour, byte minute, byte second);

void setup()
{
  Wire.begin(); // join I2C bus as a Master

  Serial.begin(9600);
  // ModTime(20,03,04,03,16,53,00);
  Serial.println("Type something to send:");
  second = clock.getSecond();
  minute = clock.getMinute();
  hour = clock.getHour(h12, PM);
  Serial.print(F("  Now Time   : ["));
  Serial.print(hour, DEC);
  Serial.print(':');
  Serial.print(minute, DEC);
  Serial.print(':');
  Serial.print(second, DEC);
  Serial.println(F(" ]"));
}

void loop()
{
  for (uint8_t i2cAddress = 1; i2cAddress < 127; i2cAddress++)
  {
    // Slave 地址 0x00~0x7F (0~127)  87 與 104 已經被 DS3231 使用
    if (i2cAddress == 87 || i2cAddress == 104)
    {
      Wire.beginTransmission(i2cAddress);
      Wire.endTransmission();
    }
    else
    {
      Wire.requestFrom(i2cAddress, DATA_SIZE);
      if (Wire.available())
      {
        i2cBufferCnt = 0;
        while (Wire.available())
        {
          i2cBuffer[i2cBufferCnt] = Wire.read();
          // '\0' as NULL, 255 = unuse index
          if (i2cBuffer[i2cBufferCnt] == '\0' || i2cBuffer[i2cBufferCnt] == 255)
            break;
          Serial.print((char)i2cBuffer[i2cBufferCnt]);
          i2cBufferCnt++;
        }
      }

      if (targetAddress == 999 || targetAddress == i2cAddress)
      {
        if (resetADN)
        {
          Wire.beginTransmission(i2cAddress);
          Wire.write("Reset/\n");
          Wire.endTransmission();
        }
         if (check)
        {
          Wire.beginTransmission(i2cAddress);
          Wire.write("Check/\n");
          Wire.endTransmission();
        }
         if (unlock)
        {
          Wire.beginTransmission(i2cAddress);
          Wire.write("Unlock/\n");
          Wire.endTransmission();
        }
        else if (resetStageLimit)
        {
          Wire.beginTransmission(i2cAddress);
          Wire.write("Stage/");
          char cstr[16];
          itoa(indexStage, cstr, 10);
          Wire.write(cstr);
          Wire.write("/");
          ltoa(timeLimit, cstr, 10);
          Wire.write(cstr);
          Wire.write("\n");
          Wire.endTransmission();
        }
      }
    }
  }
  targetAddress = -1;
  resetADN = false;
  check = false;
  resetStageLimit = false;

  // second = clock.getSecond();
  // minute = clock.getMinute();
  // hour = clock.getHour(h12, PM);
  // Serial.print(F(" *   Now Time: ["));
  // Serial.print(hour, DEC);
  // Serial.print(':');
  // Serial.print(minute, DEC);
  // Serial.print(':');
  // Serial.print(second, DEC);
  // Serial.println(F(" ]"));
  // Wire.beginTransmission(15);

  // Wire.endTransmission();

  // delay(1000);

  //  if (Serial.available()) {
  //    Wire.beginTransmission(SLAVE_ADDRESS);
  //    while(Serial.available()) {
  //      Wire.write(Serial.read());
  //      delay(1);
  //    }
  //    Wire.endTransmission();
  //  }

  //uint8_t error, i2cAddress, devCount, unCount;
  //
  //  Serial.println("Scanning...");
  //
  //  devCount = 0;
  //  unCount = 0;
  //
  //    for(i2cAddress = 1; i2cAddress < 127; i2cAddress++ )
  //  {
  //    Wire.beginTransmission(i2cAddress);
  //    error = Wire.endTransmission();
  //
  //    if (error == 0)
  //    {
  //      Serial.print("I2C device found at 0x");
  //      if (i2cAddress<16) Serial.print("0");
  //      Serial.println(i2cAddress,HEX);
  //      devCount++;
  //    }
  //    else if (error==4)
  //    {
  //      Serial.print("Unknow error at 0x");
  //      if (i2cAddress<16) Serial.print("0");
  //      Serial.println(i2cAddress,HEX);
  //      unCount++;
  //    }
  //  }
  //    if (devCount + unCount == 0)
  //    Serial.println("No I2C devices found\n");
  //  else {
  //    Serial.println();
  //    Serial.print(devCount);
  //    Serial.print(" device(s) found");
  //    if (unCount > 0) {
  //      Serial.print(", and unknown error in ");
  //      Serial.print(unCount);
  //      Serial.print(" address");
  //    }
  //    Serial.println();
  //  }
  //  delay(5000);

  //  Wire.requestFrom(SLAVE_ADDRESS, 100); // request 6 bytes from slave
  //
  //  int count = 0;
  //  while (Wire.available()) // slave may send less than requested
  //  {
  //    incomingByte = Wire.read(); // receive a byte
  //    messages[count] = incomingByte;
  //    count++;
  //    if (incomingByte == NULL)
  //      break;
  //    Serial.print(incomingByte); // print the character
  //    if (incomingByte == '\n')
  //    {
  //      for (int j = 0; j < 32; j++)
  //      {
  //        if (messages[j] == NULL)
  //          break;
  //        Serial.write(messages[j]);
  //      }
  //      return;
  //    }
  //  }
}

void ModTime(byte year, byte month, byte date, byte DoW, byte hour, byte minute, byte second)
{
  clock.setSecond(second); //Set the second
  clock.setMinute(minute); //Set the minute 设置分钟
  clock.setHour(hour);     //Set the hour 设置小时
  clock.setDoW(DoW);       //Set the day of the week 设置星期几
  clock.setDate(date);     //Set the date of the month 设置月份
  clock.setMonth(month);   //Set the month of the year 设置一年中的月份
  clock.setYear(year);     //Set the year (Last two digits of the year) 设置年份(在今年的最后两位数——比如2013年最后的13)
}

void serialEvent()
{
  // read one byte from serial port
  incomingByte = Serial.read();

  // send the received data to slave
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(incomingByte);
  Wire.endTransmission();
}

// // master_sender.ino
// // Refer to the "slave_receiver" example for use with this
// #include <Wire.h>
// #include <DS3231.h>
// void ReadDS3231();

// DS3231 Clock;
// bool Century = false;
// bool h12;
// bool PM;
// byte ADay, AHour, AMinute, ASecond, ABits;
// bool ADy, A12h, Apm;

// byte year, month, date, DoW, hour, minute, second;

// const int SLAVE_ADDRESS = 1;
// char incomingByte = 0;
// byte messages[32];
// void setup()
// {
//   Wire.begin(); // join I2C bus as a Master

//   Serial.begin(9600);
//   Serial.println("Type something to send:");

//   // Clock.setSecond(00); //Set the second
//   // Clock.setMinute(42); //Set the minute 设置分钟
//   // Clock.setHour(14);   //Set the hour 设置小时
//   // Clock.setDoW(1);     //Set the day of the week 设置星期几
//   // Clock.setDate(10);   //Set the date of the month 设置月份
//   // Clock.setMonth(2);   //Set the month of the year 设置一年中的月份
//   // Clock.setYear(20);   //Set the year (Last two digits of the year) 设置年份(在今年的最后两位数——比如2013年最后的13)

// }

// void loop()
// {
//   Wire.requestFrom(SLAVE_ADDRESS, 100); // request 6 bytes from slave

//   int count = 0;
//   while (Wire.available()) // slave may send less than requested
//   {
//     incomingByte = Wire.read(); // receive a byte
//     messages[count] = incomingByte;
//     count++;
//     if (incomingByte == NULL)
//       break;
//     Serial.print(incomingByte); // print the character
//     if (incomingByte == '\n')
//     {
//       Serial.println(count);
//       for (int j = 0; j < 32; j++)
//       {
//         if (messages[j] == NULL)
//           break;
//         Serial.write(messages[j]);
//       }
//       return;
//     }
//   }
// }

// void serialEvent()
// {
//   // read one byte from serial port
//   incomingByte = Serial.read();

//   // send the received data to slave
//   Wire.beginTransmission(SLAVE_ADDRESS);
//   Wire.write(incomingByte);
//   Wire.endTransmission();
// }

// void ReadDS3231()
// {
//   int second, minute, hour, date, month, year, temperature;
//   second = Clock.getSecond();
//   minute = Clock.getMinute();
//   hour = Clock.getHour(h12, PM);
//   date = Clock.getDate();
//   month = Clock.getMonth(Century);
//   year = Clock.getYear();

//   temperature = Clock.getTemperature();

//   Serial.print("20");
//   Serial.print(year, DEC);
//   Serial.print('-');
//   Serial.print(month, DEC);
//   Serial.print('-');
//   Serial.print(date, DEC);
//   Serial.print(' ');
//   Serial.print(hour, DEC);
//   Serial.print(':');
//   Serial.print(minute, DEC);
//   Serial.print(':');
//   Serial.print(second, DEC);
//   Serial.print('\n');
//   Serial.print("Temperature="); //这里是显示温度
//   Serial.print(temperature);
//   Serial.print('\n');
// }