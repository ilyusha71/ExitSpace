/****************************************************************************
 * Arduino Pin Setting
 * Pin      Work 
 * Pin  2 : RX = HC12 TX
 * Pin  3~: TX = HC12 RX
 * Pin  4 : SETUP = HC12 SETUP
 * Pin  5~: X
 * Pin  6~: Relay trigger in
 * Pin  7 : Buzz
 * Pin  8 : RST
 * Pin  9~: SS 2
 * Pin 10~: SS 1
 * Pin 11~: MOSI
 * Pin 12 : MISO
 * Pin 13 : SCK
 * pin A4 : SDA
 * pin A5 : SCL
 * 5V     : Vcc
 * GND    : GND
 ****************************************************************************/
#include <Arduino.h>
#include <SPI.h>
/****************************************************************************
 * RFID Setting
 ****************************************************************************/
#include <RFID.h>
#include <Reader.h>
#include <MFRC522.h>

/****************************************************************************
 * 燒錄設定
 ****************************************************************************/
String DEVICE_NAME = "2-R.2.3.4.6-A.2.4";
int callbackTime = 1000;

// #define WRITER_MODE ;
#define READER_MODE ;
// #define PRINTER ;
#ifdef READER_MODE
// #define BOX ;
#endif
#ifdef WRITER_MODE
#define NUM_RUBY 2
#endif
/****************************************************************************
 * DEFINE WRITER
 ****************************************************************************/
#ifdef WRITER_MODE
#define NR_OF_READERS 1
#define SS_PIN 10
#endif
/****************************************************************************
 * DEFINE READER
 ****************************************************************************/
#ifdef READER_MODE
#define NR_OF_RUBY 11
#define NR_OF_READERS 2
#define SS_PIN_A 10 // Slave Select Pin A
#define SS_PIN_B 9  // Slave Select Pin B
byte ssPins[] = {SS_PIN_A, SS_PIN_B};
/****************************************************************************
 * DEFINE BOX
 ****************************************************************************/
#ifdef BOX
byte bufferChallenge[18], title[18];
byte blockChallenge[2] = {6, 0}, blockTitle[2] = {6, 1};
#endif
#endif
/****************************************************************************
 * DEFINE PRINTER
 ****************************************************************************/
#ifdef PRINTER
#define NR_OF_RUBY 11
#define NR_OF_READERS 1
#define SS_PIN 10
#include "Adafruit_Thermal.h"
#include "badget.h"
// Here's the new syntax when using SoftwareSerial (e.g. Arduino Uno) ----
// If using hardware serial instead, comment out or remove these lines:

#include "SoftwareSerial.h"
#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor
#endif
/****************************************************************************
 * HC-12
 ****************************************************************************/
#include <SoftwareSerial.h>
#define SETUP 4
SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin
bool rxHasData = false;
uint8_t rxBuffer;
String rxData = "";
unsigned long callbackTimer;
/****************************************************************************
 * General
 ****************************************************************************/
READER reader[NR_OF_READERS];
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key; // 儲存金鑰
int readerIndex;
byte wakakaKey[16] = "Wakaka Key", rubyData[18];
byte bufferAgentID[18] = "Unknown";
byte blockID[2] = {7, 0};

// , bufferTime[18], bufferStage[18]
// , blockStage[2] = {7, 1}, blockTime[2] = {7, 2}
// long stageLimit[5] = {999999, 360, 780, 1200, 1500}; // 通過各關的最大時限
// long trapLimit = 180, doorLimit = 50;                //18+19 為360
// long timerStage, timerTrap, timerDoor;

#define RST_PIN 8 // Reset Pin
#define LED_BUZZER 7
#define RELAY_LOW 6
/****************************************************************************
 * Declare
 ****************************************************************************/
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

String getValue(String data, char separator, int index);
bool Clear();
bool SetCardData(byte data[], byte _block[]);
bool GetCardData(byte data[], byte _block[]);
bool ClearRuby();
bool WriteData(RFID *ruby);
bool ReadData(RFID *ruby);
void dump_byte_array(byte *buffer, byte bufferSize);
void Fail();
void AccessForbidden();
/****************************************************************************
 * Ruby Data Processor
 ****************************************************************************/
bool GetCardRubyData(int index);
bool SetCardRubyData(int index);
bool GetCardRubyData(int index)
{
  byte blockNum = (15 - (index - 1) / 3) * 4 + ((index - 1) % 3);
  byte trailerBlock = (15 - (index - 1) / 3) * 4 + 3;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  byte buffer[18];
  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, buffer, &buffersize);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  String contents = ((index < 10) ? "Ruby0" : "Ruby") + String(index);
  contents.getBytes(rubyData, 16);
  return (memcmp(buffer, rubyData, 16) == 0) ? true : false;
}
bool SetCardRubyData(int index)
{
  byte blockNum = (15 - (index - 1) / 3) * 4 + ((index - 1) % 3);
  byte trailerBlock = (15 - (index - 1) / 3) * 4 + 3;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  String contents = ((index < 10) ? "Ruby0" : "Ruby") + String(index);
  contents.getBytes(rubyData, 16);
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, rubyData, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}
/****************************************************************************
 * Send Verification
 ****************************************************************************/
void SendHeader();
void SendUnlockEvent();
void SendConferEvent();
void SendBadget(String keys);
void SendHeader()
{
  for (size_t i = 0; i < 10; i++)
  {
    if (DEVICE_NAME[i] == 0)
      break;
    HC12.print(DEVICE_NAME[i]);
  }
  readerIndex == 0 ? HC12.print(F("/0/")) : HC12.print(F("/1/"));
  for (size_t i = 0; i < 16; i++)
  {
    if (bufferAgentID[i] == 0)
      break;
    HC12.print((char)bufferAgentID[i]);
  }
}
void SendUnlockEvent()
{
  // 發送給Server確認
  SendHeader();
  HC12.print(F("/Unlock/")); // 最後一個斜線用與分離LF
  HC12.println();
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
}
void SendConferEvent()
{
  // 發送給Server確認
  SendHeader();
  HC12.print(F("/Confer/")); // 最後一個斜線用與分離LF
  HC12.println();
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
}
void SendBadget(String keys)
{
  // 發送給Server確認
  SendHeader();
  HC12.print(F("/Badget/"));
  HC12.print(keys);
  HC12.println("/"); // 最後一個斜線用與分離LF
}
/****************************************************************************
 * Receive Verification
 ****************************************************************************/
void Timeout();
void UnlockElectromagneticLock();
void ConferNewTitle();
void Timeout() // 一長音
{
  digitalWrite(LED_BUZZER, HIGH);
  delay(3000);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
}
void UnlockElectromagneticLock() // 一長三短
{
  digitalWrite(RELAY_LOW, HIGH);
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(100);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
  digitalWrite(RELAY_LOW, LOW);
}
void ConferNewTitle() // 一長三短
{
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(100);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
}

void setup()
{
  Serial.begin(9600);
  HC12.begin(9600); // Serial port to HC12
  SPI.begin();
  Serial.println(F("/****************************************************************************"));
  delay(100);

  // 準備金鑰（用於key A和key B），出廠預設為6組 0xFF。
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

#ifdef WRITER_MODE
  Serial.print(F(" * Stage "));
  Serial.println(DEVICE_NAME[0]);
  int key = getValue(getValue(DEVICE_NAME, '-', 1), '.', 1).toInt();
  reader[0].SetSpecificKey(key);
  // 初始化 MFRC522
  Serial.print(F(" *   Status ==> "));
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  mfrc522[0].PCD_DumpVersionToSerial();
#endif

#ifdef READER_MODE
  Serial.print(F(" * Stage "));
  Serial.println(DEVICE_NAME[0]);
  /****************************************************************************
   * DEFINE READER UNLOCKE CONDITION
   ****************************************************************************/
  for (int i = 0; i < NR_OF_READERS; i++)
  {
    if (i == 0) // Outer 外側
    {
      Serial.print(F(" * Outer Reader: "));
      String condition = getValue(DEVICE_NAME, '-', 1);
      Serial.println(condition);
      if (condition.charAt(0) == 'C')
      {
        int keyCount = getValue(condition, '.', 1).toInt();
        reader[i].SetKeyCount(keyCount);
      }
      else if (condition.charAt(0) == 'U')
      {
        int key = getValue(condition, '.', 1).toInt();
        reader[i].SetSpecificKey(key);
      }
      else if (condition.charAt(0) == 'R')
      {
        bool checkPassKeyCount = false;
        int checPassKeyIndex = 1;
        while (!checkPassKeyCount)
        {
          String n = getValue(condition, '.', checPassKeyIndex);
          if (n != "")
          {
            reader[i].AddPassKey(n.toInt(), checPassKeyIndex - 1);
            checPassKeyIndex++;
          }
          else
            checkPassKeyCount = true;
        }
        reader[i].SetMode(OR, checPassKeyIndex - 1);
      }
      else if (condition.charAt(0) == 'A')
      {
        bool checkPassKeyCount = false;
        int checPassKeyIndex = 1;
        while (!checkPassKeyCount)
        {
          String n = getValue(condition, '.', checPassKeyIndex);
          if (n != "")
          {
            reader[i].AddPassKey(n.toInt(), checPassKeyIndex - 1);
            checPassKeyIndex++;
          }
          else
            checkPassKeyCount = true;
        }
        reader[i].SetMode(AND, checPassKeyIndex - 1);
      }
      else if (condition.charAt(0) == 'E')
      {
        int key = getValue(condition, '.', 1).toInt();
        reader[i].SetSpecificKey(key);
        reader[i].SetMode(Special, 1);
      }
      else if (condition.charAt(0) == 'W')
        reader[i].SetMode(Wakaka, 0);

      else if (condition.charAt(0) == 'X')
        reader[i].SetMode(Disable, 0);
    }
    else if (i == 1) // Inner 內側
    {
      Serial.print(F(" * Inner Reader: "));
      String condition = getValue(DEVICE_NAME, '-', 2);
      Serial.println(condition);
      if (condition.charAt(0) == 'C')
      {
        int keyCount = getValue(condition, '.', 1).toInt();
        reader[i].SetKeyCount(keyCount);
      }
      else if (condition.charAt(0) == 'U')
      {
        int key = getValue(condition, '.', 1).toInt();
        reader[i].SetSpecificKey(key);
      }
      else if (condition.charAt(0) == 'R')
      {
        bool checkPassKeyCount = false;
        int checPassKeyIndex = 1;
        while (!checkPassKeyCount)
        {
          String n = getValue(condition, '.', checPassKeyIndex);
          if (n != "")
          {
            reader[i].AddPassKey(n.toInt(), checPassKeyIndex - 1);
            checPassKeyIndex++;
          }
          else
            checkPassKeyCount = true;
        }
        reader[i].SetMode(OR, checPassKeyIndex - 1);
      }
      else if (condition.charAt(0) == 'A')
      {
        bool checkPassKeyCount = false;
        int checPassKeyIndex = 1;
        while (!checkPassKeyCount)
        {
          String n = getValue(condition, '.', checPassKeyIndex);
          if (n != "")
          {
            reader[i].AddPassKey(n.toInt(), checPassKeyIndex - 1);
            checPassKeyIndex++;
          }
          else
            checkPassKeyCount = true;
        }
        reader[i].SetMode(AND, checPassKeyIndex - 1);
      }
      else if (condition.charAt(0) == 'E')
      {
        int key = getValue(condition, '.', 1).toInt();
        reader[i].SetSpecificKey(key);
        reader[i].SetMode(Special, 1);
      }
      else if (condition.charAt(0) == 'W')
        reader[i].SetMode(Wakaka, 0);

      else if (condition.charAt(0) == 'X')
        reader[i].SetMode(Disable, 0);
#ifdef BOX
      else if (condition.charAt(0) == 'T')
      {
        int name = getValue(condition, '.', 1).toInt();

        reader[i].SetTitle(title, name);
        Serial.print(F(" *   Target ==> ["));
        for (size_t i = 0; i < 16; i++)
        {
          if (title[i] == 0)
            break;
          Serial.write(title[i]);
        }
        Serial.println(F("]"));
      }
#endif
    }
    // 初始化 MFRC522
    Serial.print(F(" *   Status ==> "));
    mfrc522[i].PCD_Init(ssPins[i], RST_PIN); // Init each MFRC522 card
    mfrc522[i].PCD_DumpVersionToSerial();
  }
#endif
#ifdef PRINTER
  // This line is for compatibility with the Adafruit IotP project pack,
  // which uses pin 7 as a spare grounding point.  You only need this if
  // wired up the same way (w/3-pin header into pins 5/6/7):
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);

  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(9600); // Initialize SoftwareSerial
  //Serial1.begin(19200); // Use this instead if using hardware serial
  printer.begin(); // Init printer (same regardless of serial type)
  // 初始化 MFRC522
  Serial.print(F(" *   Status ==> "));
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  mfrc522[0].PCD_DumpVersionToSerial();
#endif

  pinMode(LED_BUZZER, OUTPUT);
  pinMode(RELAY_LOW, OUTPUT);
  pinMode(SETUP, OUTPUT);
  digitalWrite(LED_BUZZER, LOW);
  digitalWrite(RELAY_LOW, LOW);
  digitalWrite(SETUP, HIGH);

  DEVICE_NAME.replace(".", "");
  Serial.println(F(" *"));
  Serial.print(F(" * ["));
  Serial.print(DEVICE_NAME);
  Serial.println(F("] 2020-04-24"));
  Serial.println(F(" ****************************************************************************/"));
}

String msg;
void (*resetFunc)(void) = 0; //declare reset function at address 0
void loop()
{
  while (HC12.available())
  { // If HC-12 has data
    rxBuffer = HC12.read();
    if (rxBuffer == '\0' || rxBuffer == 255)
      break;
    rxData += (char)rxBuffer;
    if (rxBuffer == 10) // LF character
    {
      String rxHead = getValue(rxData, '/', 0);

      if (rxHead == "S") // Server Main Clock
      {
        Serial.print(rxData); // 記錄時間
        rxHasData = true;
        callbackTimer = millis() + callbackTime;
      }
      else if (rxHead == DEVICE_NAME)
      {
        Serial.print(rxData);
        String rxCommand = getValue(rxData, '/', 1);
        if (rxCommand == "Timeout")
          Timeout();
        else if (rxCommand == "Unlocked")
          UnlockElectromagneticLock();
        else if (rxCommand == "Conferred")
          ConferNewTitle();
        else if (rxCommand == "Reset")
          resetFunc();
      }
      rxData = "";
    }
  }
  if (millis() > callbackTimer && rxHasData)
  {
    rxHasData = false;
    HC12.println(DEVICE_NAME);
  }

  for (uint8_t index = 0; index < NR_OF_READERS; index++)
  {
    readerIndex = index;

    if (mfrc522[readerIndex].PICC_IsNewCardPresent() && mfrc522[readerIndex].PICC_ReadCardSerial())
    {
      // // // 特工ID 寫入
      // byte agentID[16] = "Wakaka Key";
      // if (!SetCardData(agentID, blockID))
      // {
      //   Fail();
      //   return;
      // }
      // // 如果是記錄點，寫入時間
      // byte time[16] = {hour, minute, second};
      // if (!SetCardData(time, blockTime))
      // {
      //   Fail();
      //   return;
      // }
      // 清除密鑰
      // if (!Clear)
      // {
      //   Fail();
      //   return;
      // }
      // else
      // {
      //   Serial.println(F("Clear"));
      //   //   // Halt PICC
      //   mfrc522[readerIndex].PICC_HaltA();
      //   //   // Stop encryption on PCD
      //   mfrc522[readerIndex].PCD_StopCrypto1();
      //   return;
      // }

      // 特工資料
      if (!GetCardData(bufferAgentID, blockID))
      {
        Fail();
        return;
      }
      else
      {
        Serial.print(F("Agent: ["));
        if (bufferAgentID[0] == 0)
        {
          byte unknownID[18] = "Unknown";
          memcpy(bufferAgentID, unknownID, sizeof(bufferAgentID));
        }
        for (size_t i = 0; i < 16; i++)
        {
          if (bufferAgentID[i] == 0)
            break;
          Serial.write(bufferAgentID[i]);
        }
        Serial.print(F("] ==> id: ["));
        byte *id = mfrc522[readerIndex].uid.uidByte; // 取得卡片的UID
        byte idSize = mfrc522[readerIndex].uid.size; // 取得UID的長度
        for (byte i = 0; i < idSize; i++)
        {
          Serial.print(id[i] < 16 ? " 0" : " "); // 以16進位顯示UID值
          Serial.print(id[i], HEX);
          Serial.print(F(" "));
        }
        Serial.println(F("]"));
      }
#ifdef WRITER_MODE
      Serial.print(F("  Target ==> [Ruby"));
      if (reader[index].specificKey < 10)
      {
        Serial.print(F("0"));
        Serial.print(reader[index].specificKey);
      }
      else
        Serial.print(reader[index].specificKey);
      if (SetCardRubyData(reader[index].specificKey))
      {
        Serial.println(F("] ==> OK"));
        SendUnlockEvent();
        digitalWrite(LED_BUZZER, HIGH);
        delay(500);
        digitalWrite(LED_BUZZER, LOW);
        delay(100);
        digitalWrite(LED_BUZZER, HIGH);
        delay(50);
        digitalWrite(LED_BUZZER, LOW);
        delay(50);
        digitalWrite(LED_BUZZER, HIGH);
        delay(50);
        digitalWrite(LED_BUZZER, LOW);
        delay(50);
        digitalWrite(LED_BUZZER, HIGH);
        delay(50);
        digitalWrite(LED_BUZZER, LOW);
        delay(1000);
      }
      else
      {
        Serial.println(F("] ==> Fail"));
        Fail();
      }
#endif
#ifdef PRINTER
      Serial.println(F("Printer START..."));
      bool hasKey[NR_OF_RUBY];
      for (size_t i = 1; i < NR_OF_RUBY; i++)
      {
        hasKey[i] = GetCardRubyData(i);
      }
      int countBadget = 0;
      String keys;
      printer.setDefault(); // Restore printer to defaults
      for (size_t i = 0; i < 16; i++)
      {
        if (bufferAgentID[i] == 0)
          break;
        printer.write(bufferAgentID[i]);
      }
      printer.println();
      for (size_t i = 1; i < NR_OF_RUBY; i++)
      {
        if (hasKey[i])
        {
          keys += String(i) + ".";
          countBadget++;
          switch (i)
          {
          case 1:
            // printer.printBitmap(badget_width, badget_height, Arthur);
            printer.print(F("A "));
            break;
          case 2:
            // printer.printBitmap(badget_width, badget_height, Merlin);
            printer.print(F("M "));
            break;
          case 3:
            // printer.printBitmap(badget_width, badget_height, Lancelot);
            printer.print(F("L "));
            break;
          case 4:
            // printer.printBitmap(badget_width, badget_height, Galahad);
            printer.print(F("GA "));
            break;
          case 5:
            // printer.printBitmap(badget_width, badget_height, Percival);
            printer.print(F("P "));
            break;
          case 6:
            // printer.printBitmap(badget_width, badget_height, Borse);
            if (countBadget > 5)
              printer.println();
            printer.print(F("B "));
            break;
          case 7:
            // printer.printBitmap(badget_width, badget_height, Guinevere);
            if (countBadget > 5)
              printer.println();
            printer.print(F("GU "));
            break;
          case 8:
            // printer.printBitmap(badget_width, badget_height, Excalibur);
            if (countBadget > 5)
              printer.println();
            printer.print(F("E "));
            break;
          case 9:
            // printer.printBitmap(badget_width, badget_height, SwordStone);
            if (countBadget > 5)
              printer.println();
            printer.print(F("S "));
            break;
          case 10:
            // printer.printBitmap(badget_width, badget_height, Viviane);
            if (countBadget > 5)
              printer.println();
            printer.print(F("V "));
            break;
          default:
            break;
          }
        }
      }
      if (countBadget != 0)
      {
        Serial.print(F("Printer END ===> "));
        Serial.println(keys);
        SendBadget(keys);
        printer.feed(10);
        printer.sleep();      // Tell printer to sleep
        delay(500);           // Sleep for 3 seconds
        printer.wake();       // MUST wake() before printing again, even if reset
        printer.setDefault(); // Restore printer to defaults
        delay(1000);
      }
      // 令卡片進入停止狀態
      //   // Halt PICC
      mfrc522[readerIndex].PICC_HaltA();
      //   // Stop encryption on PCD
      mfrc522[readerIndex].PCD_StopCrypto1();
      return;
#endif
      // // 關卡記錄
      // if (!GetCardData(bufferStage, blockStage))
      // {
      //   Fail();
      //   return;
      // }
      // else
      // {
      //   Serial.print(F("  STAGE:      ["));
      //   Serial.print(bufferStage[0]);
      //   Serial.print(F("] == ["));
      //   Serial.print(DEVICE_NAME[0]);
      //   Serial.print(F("]? ==> "));
      //   if (bufferStage[0] < DEVICE_NAME[0])
      //   {
      //     Serial.println(F("Cheater"));
      //     requestType = Cheater;
      //     digitalWrite(LED_BUZZER, HIGH);
      //     delay(3000);
      //     digitalWrite(LED_BUZZER, LOW);
      //     delay(1000);
      //     // 令卡片進入停止狀態
      //     //   // Halt PICC
      //     mfrc522[readerIndex].PICC_HaltA();
      //     //   // Stop encryption on PCD
      //     mfrc522[readerIndex].PCD_StopCrypto1();
      //     return;
      //   }
      //   else
      //     Serial.println(F("Go on!"));
      // }

      // // 時間記錄
      // if (!GetCardData(bufferTime, blockTime))
      // {
      //   Fail();
      //   return;
      // }
      // else
      // {
      //   Serial.print(F("  Now Time:   ["));
      //   Serial.print(hour, DEC);
      //   Serial.print(':');
      //   Serial.print(minute, DEC);
      //   Serial.print(':');
      //   Serial.print(second, DEC);
      //   Serial.println(F(" ]"));

      //   Serial.print(F("  Stage Time: ["));
      //   Serial.print(bufferTime[0], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[1], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[2], DEC);
      //   Serial.print(F("] ==> "));
      //   timerStage = (long)(hour - bufferTime[0]) * 3600 + (long)(minute - bufferTime[1]) * 60 + (long)(second - bufferTime[2]);
      //   Serial.print(timerStage);
      //   if (timerStage > stageLimit[DEVICE_NAME[0]])
      //   {
      //     Serial.println(F(" ==> TIMEOUT"));
      //     requestType = Timeout;
      //   }
      //   else
      //     Serial.println(F(" ==> GO!GO!GO!"));
      //   Serial.print(F("  Trap Time:  ["));
      //   Serial.print(bufferTime[3], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[4], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[5], DEC);
      //   Serial.print(F("] ==> "));
      //   timerTrap = (long)(hour - bufferTime[3]) * 3600 + (long)(minute - bufferTime[4]) * 60 + (long)(second - bufferTime[5]);
      //   Serial.print(timerTrap);
      //   if (timerTrap > trapLimit && bufferTime[3] != 0)
      //   {
      //     Serial.println(F(" ==> TIMEOUT"));
      //     requestType = Timeout;
      //   }
      //   else
      //     Serial.println(F(" ==> GO!GO!GO!"));
      //   Serial.print(F("  Door Time:  ["));
      //   Serial.print(bufferTime[6], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[7], DEC);
      //   Serial.print(':');
      //   Serial.print(bufferTime[8], DEC);
      //   Serial.print(F("] ==> "));
      //   timerDoor = (long)(hour - bufferTime[6]) * 3600 + (long)(minute - bufferTime[7]) * 60 + (long)(second - bufferTime[8]);
      //   Serial.print(timerDoor);
      //   if (timerDoor > doorLimit && bufferTime[6] != 0)
      //   {
      //     Serial.println(F(" ==> TIMEOUT"));
      //     requestType = Timeout;
      //   }
      //   else
      //     Serial.println(F(" ==> GO!GO!GO!"));

      //   if (requestType == Timeout)
      //   {
      //     digitalWrite(LED_BUZZER, HIGH);
      //     delay(3000);
      //     digitalWrite(LED_BUZZER, LOW);
      //     delay(1000);
      //     // 令卡片進入停止狀態
      //     //   // Halt PICC
      //     mfrc522[readerIndex].PICC_HaltA();
      //     //   // Stop encryption on PCD
      //     mfrc522[readerIndex].PCD_StopCrypto1();
      //     return;
      //   }
      // }

#ifdef READER_MODE
      // 萬能鑰匙比對
      if (memcmp(bufferAgentID, wakakaKey, 16) == 0)
      {
        Serial.println(F("==> Hello Wakaka Agent!"));
        digitalWrite(RELAY_LOW, HIGH);
        digitalWrite(LED_BUZZER, HIGH);
        delay(50);
        digitalWrite(LED_BUZZER, LOW);
        delay(50);
        digitalWrite(LED_BUZZER, HIGH);
        delay(50);
        digitalWrite(LED_BUZZER, LOW);
        delay(1000);
        digitalWrite(RELAY_LOW, LOW);
        // 令卡片進入停止狀態
        //   // Halt PICC
        mfrc522[readerIndex].PICC_HaltA();
        //   // Stop encryption on PCD
        mfrc522[readerIndex].PCD_StopCrypto1();
        return;
      }

      // 讀取器
      (readerIndex == 0) ? Serial.print(F("Outer")) : Serial.print(F("Inner"));
      Serial.println(F(" Reader:"));

      Serial.print(F("  Mode   ==> "));
      Serial.println(reader[index].modeName[reader[index].mode]);

      switch (reader[index].mode)
      {
      case Counting:
      {
        int countMatch = 0;
        for (size_t i = 1; i < NR_OF_RUBY; i++)
        {
          if (GetCardRubyData(i))
          {
            Serial.print(F("  Target ==> [Ruby"));
            if (i < 10)
            {
              Serial.print(F("0"));
              Serial.print(i);
            }
            else
              Serial.print(i);
            Serial.println(F("] ==> Match"));
            countMatch++;
          }
        }

        Serial.print(F("  Result ==> x"));
        Serial.print(countMatch);
        Serial.print(F(" / x"));
        Serial.print(reader[index].countKeys);
        Serial.print(F(" ==> "));
        if (countMatch >= reader[index].countKeys)
        {
          Serial.println(F("Unlock"));
          SendUnlockEvent();
        }
        else
        {
          Serial.println(F("AccessForbidden"));
          AccessForbidden();
        }
        break;
      }
      case Specify:
        Serial.print(F("  Target ==> [Ruby"));
        if (reader[index].specificKey < 10)
        {
          Serial.print(F("0"));
          Serial.print(reader[index].specificKey);
        }
        else
          Serial.print(reader[index].specificKey);
        if (GetCardRubyData(reader[index].specificKey))
        {
          Serial.println(F("] ==> Match"));
          SendUnlockEvent();
        }
        else
        {
          Serial.println(F("] ==> Incorrect"));
          AccessForbidden();
        }
        break;
      case OR:
      {
        int countMatch = 0;
        for (size_t i = 0; i < reader[index].countKeys; i++)
        {
          if (GetCardRubyData(reader[index].passKeys[i]))
          {
            Serial.print(F("  Target ==> [Ruby"));
            if (reader[index].passKeys[i] < 10)
            {
              Serial.print(F("0"));
              Serial.print(reader[index].passKeys[i]);
            }
            else
              Serial.print(reader[index].passKeys[i]);
            Serial.println(F("] ==> Match"));
            countMatch++;
          }
        }
        Serial.print(F("  Result ==> "));
        Serial.print(countMatch);
        Serial.print(F(" / "));
        Serial.print(reader[index].countKeys);
        Serial.print(F(" ==> "));
        if (countMatch > 0)
        {
          Serial.println(F("Unlock"));
          SendUnlockEvent();
        }
        else
        {
          Serial.println(F("AccessForbidden"));
          AccessForbidden();
        }
        break;
      }
      case AND:
      {
        int countMatch = 0;
        for (size_t i = 0; i < reader[index].countKeys; i++)
        {
          if (GetCardRubyData(reader[index].passKeys[i]))
          {
            Serial.print(F("  Target ==> [Ruby"));
            if (reader[index].passKeys[i] < 10)
            {
              Serial.print(F("0"));
              Serial.print(reader[index].passKeys[i]);
            }
            else
              Serial.print(reader[index].passKeys[i]);
            Serial.println(F("] ==> Match"));
            countMatch++;
          }
        }
        Serial.print(F("  Result ==> "));
        Serial.print(countMatch);
        Serial.print(F(" / "));
        Serial.print(reader[index].countKeys);
        Serial.print(F(" ==> "));
        if (countMatch == reader[index].countKeys)
        {
#ifndef BOX
          Serial.println(F("Unlock"));
          SendUnlockEvent();
#else
          if (!GetCardData(bufferChallenge, blockChallenge))
          {
            Fail();
            return;
          }
          else
          {
            if (bufferChallenge[0] == 0)
            {
              if (!SetCardData(title, blockChallenge))
              {
                Fail();
                return;
              }
              else
              {
                Serial.print(F("You have chosen a new challenge ["));
                for (size_t i = 0; i < 16; i++)
                {
                  if (title[i] == 0)
                    break;
                  Serial.write(title[i]);
                }
                Serial.println(F("]"));
                SendUnlockEvent();
              }
            }
            else
            {
              if (memcmp(bufferChallenge, title, 16) == 0)
              {
                Serial.print(F("This is your challenge ["));
                for (size_t i = 0; i < 16; i++)
                {
                  if (bufferChallenge[i] == 0)
                    break;
                  Serial.write(bufferChallenge[i]);
                }
                Serial.println(F("]"));
                SendUnlockEvent();
              }
              else
              {
                Serial.print(F("You already have a challenge ["));
                for (size_t i = 0; i < 16; i++)
                {
                  if (bufferChallenge[i] == 0)
                    break;
                  Serial.write(bufferChallenge[i]);
                }
                Serial.println(F("]"));
                AccessForbidden();
              }
            }
          }
#endif
        }
        else
        {
          Serial.println(F("AccessForbidden"));
          AccessForbidden();
        }
        break;
      }
      case Wakaka:
        Serial.println(F("You aren't Wakaka"));
        AccessForbidden();
        break;
      case Special:
        break;
#ifdef BOX
      case Title:
        if (!GetCardData(bufferChallenge, blockChallenge))
        {
          Fail();
          return;
        }
        else
        {
          if (memcmp(bufferChallenge, title, 16) == 0)
          {
            if (!SetCardData(title, blockTitle))
            {
              Fail();
              return;
            }
            else
            {
              Serial.print(F("Congratulations! You have got a new title ["));
              for (size_t i = 0; i < 16; i++)
              {
                if (title[i] == 0)
                  break;
                Serial.write(title[i]);
              }
              Serial.println(F("]"));
              SendConferEvent();
            }
          }
        }

        break;
#endif
      }
#endif

      // 令卡片進入停止狀態
      //   // Halt PICC
      mfrc522[readerIndex].PICC_HaltA();
      //   // Stop encryption on PCD
      mfrc522[readerIndex].PCD_StopCrypto1();
    }
  }
}

// Split Function
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
bool Clear()
{
  byte data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (size_t _sector = 1; _sector < 16; _sector++)
  {
    for (size_t _block = 0; _block < 3; _block++)
    {
      if (_sector == 7 && _block == 0)
        break;
      byte blockNum = _sector * 4 + _block; // 計算區塊的實際編號（0~63）
      byte trailerBlock = _sector * 4 + 3;  // 控制區塊編號
      status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
        return false;
      }
      status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, data, 16);
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
        return false;
      }
    }
  }
  return true;
}

bool SetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, data, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}

bool GetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, data, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}

bool ClearRuby()
{
  byte blockData[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (size_t _sector = 15; _sector >= 12; _sector--)
  {
    for (size_t _block = 0; _block < 3; _block++)
    {
      byte blockNum = _sector * 4 + _block; // 計算區塊的實際編號（0~63）
      byte trailerBlock = _sector * 4 + 3;  // 控制區塊編號
      // 驗證金鑰
      status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
      // 若未通過驗證…
      if (status != MFRC522::STATUS_OK)
      {
        // 顯示錯誤訊息
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
        return false;
      }

      // 在指定區塊寫入16位元組資料
      status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, blockData, 16);
      // 若寫入不成功…
      if (status != MFRC522::STATUS_OK)
      {
        // 顯示錯誤訊息
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
        return false;
      }
    }
  }
  return true;
}

bool WriteData(RFID *ruby)
{
  if (ruby->sector < 0 || ruby->sector > 15 || ruby->block < 0 || ruby->block > 3)
  {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return false;
  }

  byte blockNum = ruby->sector * 4 + ruby->block; // 計算區塊的實際編號（0~63）
  byte trailerBlock = ruby->sector * 4 + 3;       // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, ruby->blockData, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }

  // 顯示「寫入成功！」
  Serial.println(F("Data was written."));
  return true;
}

bool ReadData(RFID *ruby)
{
  if (ruby->sector < 0 || ruby->sector > 15 || ruby->block < 0 || ruby->block > 3)
  {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return false;
  }

  byte blockNum = ruby->sector * 4 + ruby->block; // 計算區塊的實際編號（0~63）
  byte trailerBlock = ruby->sector * 4 + 3;       // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, ruby->buffer, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void Fail()
{
#if defined WRITER_MODE || defined PRINTER
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
#else
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(100);
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
  mfrc522[readerIndex].PCD_Init(ssPins[readerIndex], RST_PIN); // Init each MFRC522 card
#endif
}
void AccessForbidden()
{
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(100);
  digitalWrite(LED_BUZZER, HIGH);
  delay(200);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
  digitalWrite(LED_BUZZER, HIGH);
  delay(200);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
}