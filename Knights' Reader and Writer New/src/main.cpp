/****************************************************************************
 * Arduino Pin Setting
 * Pin      Work 
 * Pin  2 : RX = HC12 TX
 * Pin  3~: TX = HC12 RX          
 * Pin  4 : SETUP = HC12 SETUP    白
 * Pin  5~: X
 * Pin  6~: Relay trigger in
 * Pin  7 : Buzz    紅
 * Pin  8 : SS 1    紫
 * Pin  9~: RST     白
 * Pin 10~: SS 2    藍
 * Pin 11~: MOSI    綠
 * Pin 12 : MISO    黃
 * Pin 13 : SCK     橘
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
#define ENTRY_MODE 1
#define WRITER_MODE 20
#define VIVIANE_MODE 25
#define READER_MODE 100
#define BOX_MODE 150
#define PRINTER_MODE 50
/****************************************************************************
 * 燒錄設定
 ****************************************************************************/
String DEVICE_NAME = "1-B.1-X";
// String DEVICE_NAME = "3-R.1.2.3.4.5.6.7.8.9.10-A.1.2.3.4.5.6.7.8.9.10";
// String DEVICE_NAME = "H5-C.8-C.2";
// String DEVICE_NAME = "1-N.2-X";
// String DEVICE_NAME = "4B1-A.1.2.10-T.2";
#define MODE 20
#if MODE == ENTRY_MODE
String presents = "";
boolean isPresent[11];
byte writeAgentID[16];
boolean hasNewAgentID;
#endif
#if MODE == ENTRY_MODE || MODE == WRITER_MODE
/****************************************************************************
 * DEFINE ENTRY or WRITER
 ****************************************************************************/
#define NR_OF_READERS 1
#define SS_PIN 10
#elif MODE == PRINTER_MODE
/****************************************************************************
 * DEFINE PRINTER
 ****************************************************************************/
#define NR_OF_READERS 1
#define SS_PIN 8
#include "Adafruit_Thermal.h"
#include "Badge.h"
// Here's the new syntax when using SoftwareSerial (e.g. Arduino Uno) ----
// If using hardware serial instead, comment out or remove these lines:

#include "SoftwareSerial.h"
#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor
#elif MODE >= READER_MODE || MODE == VIVIANE_MODE
/****************************************************************************
 * DEFINE READER
 ****************************************************************************/
#define NR_OF_READERS 2
#define SS_PIN_A 8  // Slave Select Pin A
#define SS_PIN_B 10 // Slave Select Pin B
byte ssPins[] = {SS_PIN_A, SS_PIN_B};
#if MODE == BOX_MODE
/****************************************************************************
 * DEFINE BOX
 ****************************************************************************/
byte bufferChallenge[18], title[18];
byte blockChallenge[2] = {6, 0}, blockTitle[2] = {6, 1};
#endif
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
int cbDelay = 100;
unsigned long cbClock;
/****************************************************************************
 * ---
 ****************************************************************************/
void (*resetFunc)(void) = 0; //declare reset function at address 0
/****************************************************************************
 * General
 ****************************************************************************/
#define NR_OF_RUBY 11
READER reader[NR_OF_READERS];
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key; // 儲存金鑰
int readerIndex;
byte wakakaKey[16] = "Wakaka Key", rubyData[18];
byte bufferAgentID[18] = "Unknown", recordAgentID[18];
byte blockID[2] = {7, 0};
boolean hasRuby[11];
unsigned long waitTimer, waitUnlock = 3000;

// , bufferTime[18], bufferStage[18]
// , blockStage[2] = {7, 1}, blockTime[2] = {7, 2}
// long stageLimit[5] = {999999, 360, 780, 1200, 1500}; // 通過各關的最大時限
// long trapLimit = 180, doorLimit = 50;                //18+19 為360
// long timerStage, timerTrap, timerDoor;

#define RST_PIN 9 // Reset Pin
#define LED_BUZZER 7
#define RELAY_LOW 6
/****************************************************************************
 * Declare
 ****************************************************************************/
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)
/****************************************************************************
 * Init
 ****************************************************************************/
boolean needInit;
void Init();
void Init()
{
  for (size_t i = 0; i < NR_OF_READERS; i++)
  {
    mfrc522[i].PCD_Init();
  }
  needInit = false;
}
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
 * Send Verification
 ****************************************************************************/
void SendHeader();
void SendResetEvent();
void SendUnlockEvent();
void SendConferEvent();
void SendBadges(String keys);
void SendHeader()
{
  HC12.print(F("Z/"));
  for (size_t i = 0; i < 32; i++)
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
void SendResetEvent(String keys)
{
  SendHeader();
  HC12.print(F("/Reset/")); // 最後一個斜線用與分離LF
  HC12.print(keys);
  HC12.println("/"); // 最後一個斜線用與分離LF
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
  needInit = true;
}
void SendBadges(String keys)
{
  SendHeader();
  HC12.print(F("/Badge/"));
  HC12.print(keys);
  HC12.println("/"); // 最後一個斜線用與分離LF
}
void SendUnlockEvent()
{
  mfrc522[readerIndex].PICC_HaltA();
  mfrc522[readerIndex].PCD_StopCrypto1();
  digitalWrite(RST_PIN, LOW);
  if (millis() < waitTimer)
    return;
  waitTimer = millis() + waitUnlock;
  SendHeader();
  HC12.print(F("/Unlock/")); // 最後一個斜線用與分離LF
  HC12.println();
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
}
void SendConferEvent()
{
  SendHeader();
  HC12.print(F("/Confer/")); // 最後一個斜線用與分離LF
  HC12.println();
  digitalWrite(LED_BUZZER, HIGH);
  delay(50);
  digitalWrite(LED_BUZZER, LOW);
}
/****************************************************************************
 * Receive Verification
 ****************************************************************************/
void Pass();
void PassSound()
{
  digitalWrite(LED_BUZZER, HIGH);
  delay(300);
  digitalWrite(LED_BUZZER, LOW);
  delay(50);
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
  delay(500);
}
void Timeout();
void Timeout() // 一長音
{
  digitalWrite(LED_BUZZER, HIGH);
  delay(3000);
  digitalWrite(LED_BUZZER, LOW);
  delay(500);
  needInit = true;
}
#if MODE == ENTRY_MODE
void Present();
void Present()
{
  for (size_t index = 1; index < NR_OF_RUBY; index++)
  {
    isPresent[index] = false;
  }
  bool checkPresentCount = false;
  int presentIndex = 0;
  while (!checkPresentCount)
  {
    String n = Split(presents, '.', presentIndex);
    if (n != "")
    {
      isPresent[n.toInt()] = true;
      presentIndex++;
    }
    else
      checkPresentCount = true;
  }
}
#elif MODE >= READER_MODE
void UnlockForce();
void UnlockEML();
void UnlockEML_1_U1_X();
void UnlockEML_3_E6_E4(int index);
void ConferNewTitle();
void UnlockForce()
{
  waitTimer = millis();
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  PassSound();
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  needInit = true;
}
void UnlockEML() // 一長三短
{
  waitTimer = millis();
  digitalWrite(RELAY_LOW, HIGH);
  PassSound();
  digitalWrite(RELAY_LOW, LOW);
  needInit = true;
}
void UnlockEML_1_U1_X() // 一長三短
{
  waitTimer = millis();
  digitalWrite(RELAY_LOW, HIGH);
  PassSound();
}
void UnlockEML_3_E6_E4(int index) // 一長三短
{
  index = 6 - index;
  waitTimer = millis();
  digitalWrite(index, HIGH);
  PassSound();
  digitalWrite(index, LOW);
  needInit = true;
}
void ConferNewTitle() // 一長三短
{
  PassSound();
  needInit = true;
}
#endif
/****************************************************************************
 * Ruby Data Processor
 ****************************************************************************/
void Fail();
void AccessForbidden();
#if MODE == ENTRY_MODE
bool ResetNewAgent();
bool ResetNewAgent()
{
  byte data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (size_t _sector = 1; _sector < 16; _sector++)
  {
    for (size_t _block = 0; _block < 3; _block++)
    {
      if (_sector == 7 && _block == 0) // 記錄玩家代號的區塊
        break;
      byte blockNum = _sector * 4 + _block; // 計算區塊的實際編號（0~63）
      byte trailerBlock = _sector * 4 + 3;  // 控制區塊編號
      status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
      if (status != MFRC522::STATUS_OK)
      {
        Fail();
        return false;
      }

      int index = (15 - _sector) * 3 + _block + 1;
      if (index > 0 && index <= 10)
      {
        if (isPresent[index])
        {
          String contents = ((index < 10) ? "Ruby0" : "Ruby") + String(index);
          contents.getBytes(rubyData, 16);
          status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, rubyData, 16);
          if (status != MFRC522::STATUS_OK)
          {
            Fail();
            return false;
          }
          continue;
        }
      }
      status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, data, 16);
      if (status != MFRC522::STATUS_OK)
      {
        Fail();
        return false;
      }
    }
  }
  Serial.println(F("CLEAR"));
  return true;
}
#endif
bool SetCardData(byte data[], byte _block[]);
bool GetCardData(byte data[], byte _block[]);
bool GetCardRubyData(int index);
bool SetCardRubyData(int index);
int GetAllCardRubyData();
bool SetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號
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
  return true;
}
bool GetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, data, &buffersize);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}
bool GetCardRubyData(int index)
{
  byte blockNum = (15 - (index - 1) / 3) * 4 + ((index - 1) % 3);
  byte trailerBlock = (15 - (index - 1) / 3) * 4 + 3;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
  if (status != MFRC522::STATUS_OK)
  {
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  byte buffer[18];
  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, buffer, &buffersize);
  if (status != MFRC522::STATUS_OK)
  {
    // Serial.print(F("MIFARE_read() failed: "));
    // Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
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
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  String contents = ((index < 10) ? "Ruby0" : "Ruby") + String(index);
  contents.getBytes(rubyData, 16);
  status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Write(blockNum, rubyData, 16);
  if (status != MFRC522::STATUS_OK)
  {
    // Serial.print(F("MIFARE_Write() failed: "));
    // Serial.println(mfrc522[readerIndex].GetStatusCodeName(status));
    return false;
  }
  return true;
}
int GetAllCardRubyData()
{
  int count = 0;
  for (size_t index = 1; index < NR_OF_RUBY; index++)
  {
    byte blockNum = (15 - (index - 1) / 3) * 4 + ((index - 1) % 3);
    byte trailerBlock = (15 - (index - 1) / 3) * 4 + 3;
    status = (MFRC522::StatusCode)mfrc522[readerIndex].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerIndex].uid));
    if (status != MFRC522::STATUS_OK)
      continue;
    byte buffer[18];
    byte buffersize = 18;
    status = (MFRC522::StatusCode)mfrc522[readerIndex].MIFARE_Read(blockNum, buffer, &buffersize);
    if (status != MFRC522::STATUS_OK)
      continue;
    String contents = ((index < 10) ? "Ruby0" : "Ruby") + String(index);
    contents.getBytes(rubyData, 16);
    count = (memcmp(buffer, rubyData, 16) == 0) ? count + 1 : count + 0;
  }
  return count;
}
void Fail()
{
  mfrc522[readerIndex].PICC_HaltA();
  mfrc522[readerIndex].PCD_StopCrypto1();
  digitalWrite(RST_PIN, LOW);
  // #if MODE == ENTRY_MODE || MODE == WRITER_MODE || MODE == PRINTER_MODE
  //   mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  // #else
  //   // digitalWrite(LED_BUZZER, HIGH);
  //   // delay(50);
  //   // digitalWrite(LED_BUZZER, LOW);
  //   // delay(10);
  //   // digitalWrite(LED_BUZZER, HIGH);
  //   // delay(50);
  //   // digitalWrite(LED_BUZZER, LOW);
  //   // delay(10);
  //   mfrc522[readerIndex].PCD_Init(ssPins[readerIndex], RST_PIN); // Init each MFRC522 card
  // #endif
}
void AccessForbidden()
{
  mfrc522[readerIndex].PICC_HaltA();
  mfrc522[readerIndex].PCD_StopCrypto1();
  digitalWrite(RST_PIN, LOW);
  // digitalWrite(LED_BUZZER, HIGH);
  // delay(50);
  // digitalWrite(LED_BUZZER, LOW);
  // delay(10);
  // digitalWrite(LED_BUZZER, HIGH);
  // delay(50);
  // digitalWrite(LED_BUZZER, LOW);
  needInit = true;
}
/****************************************************************************
 * Printer
 ****************************************************************************/
#if MODE == PRINTER_MODE
void Print();
void Print()
{
  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(9600); // Initialize SoftwareSerial
  printer.begin();      // Init printer (same regardless of serial type)
  Serial.println(F("Printer START..."));
  bool hasKey[NR_OF_RUBY];
  for (size_t i = 1; i < NR_OF_RUBY; i++)
  {
    hasKey[i] = GetCardRubyData(i);
  }
  mfrc522[readerIndex].PICC_HaltA();
  int countBadge = 0;
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
      countBadge++;
      switch (i)
      {
      case 1:
        // printer.printBitmap(Badge_width, Badge_height, Arthur);
        printer.print(F("A "));
        break;
      case 2:
        // printer.printBitmap(Badge_width, Badge_height, Merlin);
        printer.print(F("M "));
        break;
      case 3:
        // printer.printBitmap(Badge_width, Badge_height, Lancelot);
        printer.print(F("L "));
        break;
      case 4:
        // printer.printBitmap(Badge_width, Badge_height, Galahad);
        printer.print(F("GA "));
        break;
      case 5:
        // printer.printBitmap(Badge_width, Badge_height, Percival);
        printer.print(F("P "));
        break;
      case 6:
        // printer.printBitmap(Badge_width, Badge_height, Borse);
        printer.print(F("B "));
        break;
      case 7:
        // printer.printBitmap(Badge_width, Badge_height, Guinevere);
        printer.print(F("GU "));
        break;
      case 8:
        // printer.printBitmap(Badge_width, Badge_height, Excalibur);
        printer.print(F("E "));
        break;
      case 9:
        // printer.printBitmap(Badge_width, Badge_height, SwordStone);
        printer.print(F("S "));
        break;
      case 10:
        // printer.printBitmap(Badge_width, Badge_height, Viviane);
        printer.print(F("V "));
        break;
      default:
        break;
      }
      if (countBadge == 5)
        printer.println();
    }
  }
  if (countBadge != 0)
  {
    Serial.print(F("Printer END ===> "));
    Serial.println(keys);
    SendBadges(keys);
    printer.feed(5);
    printer.sleep();      // Tell printer to sleep
    delay(500);           // Sleep for 3 seconds
    printer.wake();       // MUST wake() before printing again, even if reset
    printer.setDefault(); // Restore printer to defaults
    delay(1000);
  }
  mfrc522[readerIndex].PCD_StopCrypto1();
  digitalWrite(RST_PIN, LOW);
}
#endif

void setup()
{
  Serial.begin(9600);
  HC12.begin(9600); // Serial port to HC12
  SPI.begin();
  // 準備金鑰（用於key A和key B），出廠預設為6組 0xFF。
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("/****************************************************************************"));
  Serial.print(F(" * Stage "));
  Serial.println(DEVICE_NAME[0]);
#if MODE == WRITER_MODE
  int key = Split(Split(DEVICE_NAME, '-', 1), '.', 1).toInt();
   reader[0].AddPassKey(key);
#elif MODE == VIVIANE_MODE
  int key = Split(Split(DEVICE_NAME, '-', 1), '.', 1).toInt();
  reader[0].SetSpecificKey(key);
  for (int i = 0; i < NR_OF_READERS; i++)
  {
    Serial.print(F(" *   Status ==> "));
    mfrc522[i].PCD_Init(ssPins[i], RST_PIN); // Init each MFRC522 card
    mfrc522[i].PCD_DumpVersionToSerial();
  }
#elif MODE >= READER_MODE
  /****************************************************************************
   * DEFINE READER UNLOCKE CONDITION
   ****************************************************************************/
  for (int i = 0; i < NR_OF_READERS; i++)
  {
    if (i == 0)
      Serial.print(F(" * Outer Reader: "));
    else if (i == 1)
      Serial.print(F(" * Inner Reader: "));

    String condition = Split(DEVICE_NAME, '-', i + 1);
    Serial.println(condition);
    bool checkPassKeyCount = false;
    int checPassKeyIndex = 1;
    while (!checkPassKeyCount)
    {
      String n = Split(condition, '.', checPassKeyIndex);
      if (n != "")
      {
        reader[i].AddPassKey(n.toInt());
        checPassKeyIndex++;
      }
      else
        checkPassKeyCount = true;
    }

    switch (condition.charAt(0))
    {
#if MODE == BOX_MODE
    case 'T':
      int name = Split(condition, '.', 1).toInt();
      reader[i].SetTitle(title, name);
      Serial.print(F(" *   Target ==> ["));
      for (size_t i = 0; i < 16; i++)
      {
        if (title[i] == 0)
          break;
        Serial.write(title[i]);
      }
      Serial.println(F("]"));
      break;
#endif
    case 'C':
      reader[i].SetKeyCount(Split(condition, '.', 1).toInt());
      break;
    case 'W':
      reader[i].SetMode(Wakaka);
      break;
    case 'X':
      reader[i].SetMode(Disable);
      break;
    case 'U':
      reader[i].SetMode(Specify);
      break;
    case 'E':
      reader[i].SetMode(Special);
      pinMode(5, OUTPUT);
      digitalWrite(5, LOW);
      break;
    case 'R':
      reader[i].SetMode(OR);
      break;
    case 'A':
      reader[i].SetMode(AND);
      break;
    }

    if (reader[i].mode != Disable)
    {
      // 初始化 MFRC522
      Serial.print(F(" *   Status ==> "));
      mfrc522[i].PCD_Init(ssPins[i], RST_PIN); // Init each MFRC522 card
      mfrc522[i].PCD_DumpVersionToSerial();
    }
  }
#elif MODE == PRINTER_MODE
  // This line is for compatibility with the Adafruit IotP project pack,
  // which uses pin 7 as a spare grounding point.  You only need this if
  // wired up the same way (w/3-pin header into pins 5/6/7):
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);

  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(9600); // Initialize SoftwareSerial
  //Serial1.begin(19200); // Use this instead if using hardware serial
  printer.begin(); // Init printer (same regardless of serial type)
#endif
#if MODE == ENTRY_MODE || MODE == WRITER_MODE || MODE == PRINTER_MODE
  // 初始化 MFRC522
  Serial.print(F(" *   Status ==> "));
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  mfrc522[0].PCD_DumpVersionToSerial();
#endif

  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW); // mfrc522 readers hard power down.
  pinMode(LED_BUZZER, OUTPUT);
  pinMode(RELAY_LOW, OUTPUT);
  pinMode(SETUP, OUTPUT);
  digitalWrite(LED_BUZZER, LOW);
  digitalWrite(RELAY_LOW, LOW);
  digitalWrite(SETUP, HIGH);

  DEVICE_NAME.replace(".", "");
#if MODE == READER_MODE
  if (DEVICE_NAME == "1-U1-X")
    pinMode(5, INPUT_PULLUP);
#endif
  Serial.println(F(" *"));
  Serial.print(F(" * ["));
  Serial.print(DEVICE_NAME);
  Serial.println(F("] 2020-04-24"));
  Serial.println(F(" ****************************************************************************/"));
}

void loop()
{
#if MODE == READER_MODE
  if (DEVICE_NAME == "1-U1-X") // 磁簧復位
  {
    if (digitalRead(5) == 0)
      digitalWrite(RELAY_LOW, LOW);
  }
#endif
#if MODE == PRINTER_MODE
  HC12.begin(9600);
#endif
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
        needInit = true;
      }
      else if (rxHead == DEVICE_NAME)
      {
        Serial.print(rxData);
        String rxCommand = Split(rxData, '/', 2);
        if (rxCommand == "Checking")
        {
          rxHasData = true;
          cbDelay = Split(rxData, '/', 3).toInt();
        }
        else if (rxCommand == "Init")
          Init();
        else if (rxCommand == "Timeout")
          Timeout();
#if MODE == ENTRY_MODE
        else if (rxCommand == "Present")
        {
          presents = Split(rxData, '/', 3);
          Present();
        }
        else if (rxCommand == "ID")
        {
          hasNewAgentID = true;
          Split(rxData, '/', 3).getBytes(writeAgentID, 16);
        }
#elif MODE == READER_MODE
        else if (rxCommand == "UnlockForce")
          UnlockForce();
        else if (rxCommand == "Unlocked")
          UnlockEML();
        else if (rxCommand == "Unlocked_1_U1_X")
          UnlockEML_1_U1_X();
        else if (rxCommand == "Unlocked_3_E6_E4")
          UnlockEML_3_E6_E4(Split(rxData, '/', 4).toInt());
#if MODE == BOX_MODE
        else if (rxCommand == "Conferred")
          ConferNewTitle();
#endif
#endif
        else if (rxCommand == "Reset")
          resetFunc();
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

  if (needInit)
    Init();
  for (uint8_t index = 0; index < NR_OF_READERS; index++)
  {
    digitalWrite(RST_PIN, HIGH);
    mfrc522[index].PCD_Init();
    readerIndex = index;
    if (reader[index].mode == Disable)
      continue;
    if (mfrc522[readerIndex].PICC_IsNewCardPresent() && mfrc522[readerIndex].PICC_ReadCardSerial())
    {
#if MODE == VIVIANE_MODE
      if (readerIndex == 1)
      {
        Serial.println(F("Viviane Punishment"));
        mfrc522[readerIndex].PICC_HaltA();
        digitalWrite(RELAY_LOW, HIGH);
        delay(1000);
        digitalWrite(RELAY_LOW, LOW);
        mfrc522[readerIndex].PCD_StopCrypto1();
        digitalWrite(RST_PIN, LOW);
        continue;
      }
#endif
      // // 如果是記錄點，寫入時間
      // byte time[16] = {hour, minute, second};
      // if (!SetCardData(time, blockTime))
      // {
      //   Fail();
      //   continue;
      // }
      // 特工資料
      if (!GetCardData(bufferAgentID, blockID))
      {
        Fail();
        continue;
      }
      else
      {
        if (memcmp(bufferAgentID, wakakaKey, 16) == 0)
        {
          mfrc522[readerIndex].PICC_HaltA();
          Serial.println(F("==> Hello Wakaka Agent!"));
#if MODE >= READER_MODE
          UnlockForce();
#endif
          mfrc522[readerIndex].PCD_StopCrypto1();
          digitalWrite(RST_PIN, LOW);
          continue;
        }
        if (!memcmp(bufferAgentID, recordAgentID, 16) == 0)
        {
          for (size_t i = 0; i < 18; i++)
          {
            recordAgentID[i] = bufferAgentID[i];
          }
          for (size_t i = 0; i < NR_OF_RUBY; i++)
          {
            hasRuby[i] = false;
          }
        }
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
#if MODE == ENTRY_MODE
      /****************************************************************************
       * Agent ID Write
       * 需透過Serve開啟
       ****************************************************************************/
      if (hasNewAgentID)
      {
        if (!SetCardData(writeAgentID, blockID))
        {
          Fail();
          continue;
        }
        else
          hasNewAgentID = false;
      }
      if (ResetNewAgent())
        SendResetEvent(presents);
      mfrc522[readerIndex].PICC_HaltA();
      delay(300);
      mfrc522[readerIndex].PCD_StopCrypto1();
      digitalWrite(RST_PIN, LOW);
#elif MODE == WRITER_MODE || MODE == VIVIANE_MODE
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
        mfrc522[readerIndex].PICC_HaltA();
        Serial.println(F("] ==> OK"));
        SendBadges(String(reader[index].specificKey) + ".");
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
        mfrc522[readerIndex].PCD_StopCrypto1();
        digitalWrite(RST_PIN, LOW);
      }
      else
      {
        Serial.println(F("] ==> Fail"));
        Fail();
        continue;
      }
#elif MODE == PRINTER_MODE
      Print();
      continue;
#elif MODE >= READER_MODE
      // 讀取器
      (readerIndex == 0) ? Serial.print(F("Outer")) : Serial.print(F("Inner"));
      Serial.println(F(" Reader:"));
      Serial.print(F("  Mode   ==> "));
      Serial.println(reader[index].Pass(reader[index].mode));
      switch (reader[index].mode)
      {
      case Disable:
        break;
      case Counting:
      {
        int countMatch = 0;
        for (size_t i = 1; i < NR_OF_RUBY; i++)
        {
          if (hasRuby[i])
            continue;
          hasRuby[i] = GetCardRubyData(i);
        }
        for (size_t k = 1; k < NR_OF_RUBY; k++)
        {
          if (hasRuby[k])
          {
            countMatch++;
            Serial.print(F("  Target ==> [Ruby"));
            if (k < 10)
              Serial.print(F("0"));
            Serial.print(k);
            Serial.println(F("] ==> Match"));
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
      case Special:
        Serial.print(F("  Target ==> [Ruby"));
        if (reader[index].specificKey < 10)
          Serial.print(F("0"));
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
        for (int key = 1; key < NR_OF_RUBY; key++)
        {
          if (!reader[index].passKeys[key] || hasRuby[key])
            continue;
          hasRuby[key] = GetCardRubyData(key);
        }
        for (int key = 1; key < NR_OF_RUBY; key++)
        {
          if (reader[index].passKeys[key] && hasRuby[key])
          {
            countMatch++;
            Serial.print(F("  Target ==> [Ruby"));
            if (key < 10)
              Serial.print(F("0"));
            Serial.print(key);
            Serial.println(F("] ==> Match"));
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
        for (int key = 1; key < NR_OF_RUBY; key++)
        {
          if (!reader[index].passKeys[key] || hasRuby[key])
            continue;
          hasRuby[key] = GetCardRubyData(key);
        }
        for (int key = 1; key < NR_OF_RUBY; key++)
        {
          if (reader[index].passKeys[key] && hasRuby[key])
          {
            countMatch++;
            Serial.print(F("  Target ==> [Ruby"));
            if (key < 10)
              Serial.print(F("0"));
            Serial.print(key);
            Serial.println(F("] ==> Match"));
          }
        }

        Serial.print(F("  Result ==> "));
        Serial.print(countMatch);
        Serial.print(F(" / "));
        Serial.print(reader[index].countKeys);
        Serial.print(F(" ==> "));
        if (countMatch == reader[index].countKeys)
        {
#if MODE != BOX_MODE
          Serial.println(F("Unlock"));
          SendUnlockEvent();
#else
          if (!GetCardData(bufferChallenge, blockChallenge))
          {
            Fail();
            continue;
          }
          else
          {
            if (bufferChallenge[0] == 0)
            {
              if (!SetCardData(title, blockChallenge))
              {
                Fail();
                continue;
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
      case Title:
#if MODE == BOX_MODE
        if (!GetCardData(bufferChallenge, blockChallenge))
        {
          Fail();
          continue;
        }
        else
        {
          if (memcmp(bufferChallenge, title, 16) == 0)
          {
            if (!SetCardData(title, blockTitle))
            {
              Fail();
              continue;
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
#endif
        break;
      }
      // mfrc522[readerIndex].PICC_HaltA();
      // mfrc522[readerIndex].PCD_StopCrypto1();
#endif
    }
  }
}