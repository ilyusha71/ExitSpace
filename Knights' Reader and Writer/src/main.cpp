/****************************************************************************
 * RFID Tag Index
 * Wall
 * byte targetTag[2][10] =
    {
        {0x3d, 0x0c, 0x10, 0x2b},   // Diamond 7
        {0xf9, 0x35, 0x47, 0x0d}};  // Club 7
 ****************************************************************************/
/****************************************************************************
 * Arduino Pin Setting
 * Pin      Work 
 * Pin  2 : Output 1 LOW ==> [NO] output (Module)
 * Pin  3~: Output 2 LOW ==> [NO] output (Module)
 * Pin  4 : X
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
/****************************************************************************
 * RFID Card Code
 *    SS 1 Round 1 ==> Heart J
 *    Main:
 * Reserve:
 *    SS 2 Round 7 ==> Heart 7
 *    Main:
 * Reserve:
 *    SS 3 Round12 ==> Heart A
 *    Main:
 * Reserve:
 *    SS 4 ==> Joker
 *    Main:
 * Reserve:
 ****************************************************************************/
#include <Arduino.h>
#include <SPI.h>
/****************************************************************************
 * RFID Setting
 ****************************************************************************/
#include <RFID.h>
#include <Reader.h>
#include <MFRC522.h>
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;           // 儲存金鑰
byte wakakaKey[16] = "Wakaka Key"; // 最多可存入16個字元
#define WRITER_MODE ;
#ifdef WRITER_MODE
RFID rfid;
#define NR_OF_READERS 1
#define SS_PIN 10
#endif
// #define READER_MODE ;
#ifdef READER_MODE
#define NR_OF_RUBY 11
RFID rfid[NR_OF_RUBY];
#define NR_OF_READERS 2
#define SS_PIN_A 10 // Slave Select Pin A
#define SS_PIN_B 9  // Slave Select Pin B
byte ssPins[] = {SS_PIN_A, SS_PIN_B};
#endif
READER reader[NR_OF_READERS];
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
int readerSelected;
byte bufferAgentID[18] = "Unknown", bufferTime[18], bufferStage[18];
byte blockID[2] = {7, 0}, blockStage[2] = {7, 1}, blockTime[2] = {7, 2};
long stageLimit[5] = {999999, 360, 780, 1200, 1500}; // 通過各關的最大時限
long trapLimit = 180, doorLimit = 50;
long timerStage, timerTrap, timerDoor;

#define RST_PIN 8 // Reset Pin
#define LED_BUZZER 7
#define RELAY_LOW 6

/****************************************************************************
 * 燒錄設定
 ****************************************************************************/
#define STAGE 1          // 所在關卡
#define SLAVE_ADDRESS 5  // Slave 地址 0x00~0x7F (0~127)  87 與 104 已經被 DS3231 使用
char DEVICE_NAME[] = ""; // 讀取器名字需定義，寫入器名稱為Badge名稱

// I2C & DS3231
#include <DS3231.h> // 包含 Wire.h
#define I2C_BUFFER_SIZE 32
uint8_t i2cBuffer[I2C_BUFFER_SIZE];
uint8_t i2cBufferCnt = 0;
enum RequestType
{
  None,
  Cheacking,
  Timeout,
  Get,
  Pass,
  Forbidden,
  Cheater,
} requestType = None;
bool hasNewMsg;
void receiveEvent(int howMany);
void requestEvent();
DS3231 clock;
bool Century = false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;
byte year, month, date, DoW, hour, minute, second;

#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

String getValue(String data, char separator, int index);
bool SetCardData(byte data[], byte _block[]);
bool GetCardData(byte data[], byte _block[]);
bool ClearRuby();
bool WriteData(RFID *ruby);
bool ReadData(RFID *ruby);
void dump_byte_array(byte *buffer, byte bufferSize);
void Fail();
void Unlock();
void AccessForbidden();

void setup()
{
  Wire.begin(SLAVE_ADDRESS);    // join I2C bus as a slave with address 1
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event

  Serial.begin(9600);
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
  Serial.println(STAGE);
  // 定義 RFID 寫入區域與內容
  Serial.print(F(" *  "));
  rfid.Initialize(SLAVE_ADDRESS);
  Serial.print(F(" Badge Writer in [Sector "));
  Serial.print(rfid.sector);
  Serial.print(F("] [Block "));
  Serial.print(rfid.block);
  Serial.println(F("]"));
  // 初始化 MFRC522
  Serial.print(F(" *   Status ==> "));
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  mfrc522[0].PCD_DumpVersionToSerial();
#endif

#ifdef READER_MODE
  Serial.print(F(" * Stage "));
  Serial.println(STAGE);
  for (int i = 1; i < NR_OF_RUBY; i++)
  {
    // 定義 RFID 讀取區域與內容
    Serial.print(F(" *  "));
    rfid[i].Initialize(i);
    Serial.print(F(" Badge Reader in [Sector "));
    Serial.print(rfid[i].sector);
    Serial.print(F("] [Block "));
    Serial.print(rfid[i].block);
    Serial.println(F("]"));
  }

  for (int i = 0; i < NR_OF_READERS; i++)
  {
    // 定義 Reader Mode
    if (i == 0) // Outer 外側
    {
      Serial.println(F(" * Outer Reader:"));
      reader[i].Initialize(1); // 計數密鑰
      // reader[i].Initialize(rfid[4]); // 指定密鑰 Pass
      // RFID *keys[] = {&rfid[2], &rfid[4], &rfid[6]}; // 擇一/多重密鑰 Pass
      // reader[i].Initialize(AND, keys);
    }
    else if (i == 1) // Inner 內側
    {
      Serial.println(F(" * Inner Reader:"));
      reader[i].Initialize(1); // 計數密鑰
      // reader[i].Initialize(rfid[6]); // // 指定密鑰 Pass
      // RFID *keys[] = {&rfid[4], &rfid[6]}; // 擇一/多重密鑰 Pass
      // reader[i].Initialize(AND, keys);
    }
    // 初始化 MFRC522
    Serial.print(F(" *   Status ==> "));
    mfrc522[i].PCD_Init(ssPins[i], RST_PIN); // Init each MFRC522 card
    mfrc522[i].PCD_DumpVersionToSerial();
  }
#endif

  pinMode(LED_BUZZER, OUTPUT);
  pinMode(RELAY_LOW, OUTPUT);
  digitalWrite(LED_BUZZER, LOW);
  digitalWrite(RELAY_LOW, LOW);

  Serial.println(F(" *"));
  Serial.println(F(" * 2020-02-14_Please scan MIFARE Classic card..."));
  Serial.println(F(" ****************************************************************************/"));

  // pinMode(RST_PIN, OUTPUT);
  // digitalWrite(RST_PIN, LOW);

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
String msg;
void (*resetFunc)(void) = 0; //declare reset function at address 0
void loop()
{
  if (hasNewMsg)
  {
    hasNewMsg = false;
    Serial.print(F("Server Messages: "));
    Serial.print(msg);
    String msgType = getValue(msg, '/', 0);
    String msgIndex = getValue(msg, '/', 1);
    String msgValue = getValue(msg, '/', 2);
    if (msgType == "Reset")
    {
      delay(500);
      resetFunc();
    }
    else if (msgType == "Check")
    {
      requestType = Cheacking;
      return;
    }
    else if (msgType == "Stage")
      stageLimit[msgIndex.toInt()] = msgValue.toInt();

    Serial.println(F("==================================================="));
    return;
  }

  for (uint8_t index = 0; index < NR_OF_READERS; index++)
  {
    readerSelected = index;

    // Look for new cards
    if (mfrc522[readerSelected].PICC_IsNewCardPresent() && mfrc522[readerSelected].PICC_ReadCardSerial())
    {
      // 時鐘
      second = clock.getSecond();
      minute = clock.getMinute();
      hour = clock.getHour(h12, PM);

      // // 特工ID 寫入
      // byte agentID[16] = "KGB020";
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
      // // 清除密鑰
      // if (!ClearRuby())
      // {
      //   Fail();
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
        byte *id = mfrc522[readerSelected].uid.uidByte; // 取得卡片的UID
        byte idSize = mfrc522[readerSelected].uid.size; // 取得UID的長度
        for (byte i = 0; i < idSize; i++)
        {
          Serial.print(id[i] < 16 ? " 0" : " "); // 以16進位顯示UID值
          Serial.print(id[i], HEX);
          Serial.print(F(" "));
        }
        Serial.println(F("]"));
      }

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
        return;
      }

      // 讀取器
      (readerSelected == 0) ? Serial.print(F("Outer")) : Serial.print(F("Inner"));
      Serial.println(F(" Reader:"));
#endif

      // 關卡記錄
      if (!GetCardData(bufferStage, blockStage))
      {
        Fail();
        return;
      }
      else
      {
        Serial.print(F("  STAGE:      ["));
        Serial.print(bufferStage[0]);
        Serial.print(F("] == ["));
        Serial.print(STAGE);
        Serial.print(F("]? ==> "));
        if (bufferStage[0] < STAGE)
        {
          Serial.println(F("Cheater"));
          requestType = Cheater;
          digitalWrite(LED_BUZZER, HIGH);
          delay(3000);
          digitalWrite(LED_BUZZER, LOW);
          delay(1000);
          // 令卡片進入停止狀態
          //   // Halt PICC
          mfrc522[readerSelected].PICC_HaltA();
          //   // Stop encryption on PCD
          mfrc522[readerSelected].PCD_StopCrypto1();
          return;
        }
        else
          Serial.println(F("Go on!"));
      }

      // 時間記錄
      if (!GetCardData(bufferTime, blockTime))
      {
        Fail();
        return;
      }
      else
      {
        Serial.print(F("  Now Time:   ["));
        Serial.print(hour, DEC);
        Serial.print(':');
        Serial.print(minute, DEC);
        Serial.print(':');
        Serial.print(second, DEC);
        Serial.println(F(" ]"));

        Serial.print(F("  Stage Time: ["));
        Serial.print(bufferTime[0], DEC);
        Serial.print(':');
        Serial.print(bufferTime[1], DEC);
        Serial.print(':');
        Serial.print(bufferTime[2], DEC);
        Serial.print(F("] ==> "));
        timerStage = (long)(hour - bufferTime[0]) * 3600 + (long)(minute - bufferTime[1]) * 60 + (long)(second - bufferTime[2]);
        Serial.print(timerStage);
        if (timerStage > stageLimit[STAGE])
        {
          Serial.println(F(" ==> TIMEOUT"));
          requestType = Timeout;
        }
        else
          Serial.println(F(" ==> GO!GO!GO!"));
        Serial.print(F("  Trap Time:  ["));
        Serial.print(bufferTime[3], DEC);
        Serial.print(':');
        Serial.print(bufferTime[4], DEC);
        Serial.print(':');
        Serial.print(bufferTime[5], DEC);
        Serial.print(F("] ==> "));
        timerTrap = (long)(hour - bufferTime[3]) * 3600 + (long)(minute - bufferTime[4]) * 60 + (long)(second - bufferTime[5]);
        Serial.print(timerTrap);
        if (timerTrap > trapLimit && bufferTime[3] != 0)
        {
          Serial.println(F(" ==> TIMEOUT"));
          requestType = Timeout;
        }
        else
          Serial.println(F(" ==> GO!GO!GO!"));
        Serial.print(F("  Door Time:  ["));
        Serial.print(bufferTime[6], DEC);
        Serial.print(':');
        Serial.print(bufferTime[7], DEC);
        Serial.print(':');
        Serial.print(bufferTime[8], DEC);
        Serial.print(F("] ==> "));
        timerDoor = (long)(hour - bufferTime[6]) * 3600 + (long)(minute - bufferTime[7]) * 60 + (long)(second - bufferTime[8]);
        Serial.print(timerDoor);
        if (timerDoor > doorLimit && bufferTime[6] != 0)
        {
          Serial.println(F(" ==> TIMEOUT"));
          requestType = Timeout;
        }
        else
          Serial.println(F(" ==> GO!GO!GO!"));

        if (requestType == Timeout)
        {
          digitalWrite(LED_BUZZER, HIGH);
          delay(3000);
          digitalWrite(LED_BUZZER, LOW);
          delay(1000);
          // 令卡片進入停止狀態
          //   // Halt PICC
          mfrc522[readerSelected].PICC_HaltA();
          //   // Stop encryption on PCD
          mfrc522[readerSelected].PCD_StopCrypto1();
          return;
        }
      }

#ifdef WRITER_MODE
      Serial.print(F("  Target ==> "));
      rfid.ShowRubyName();
      Serial.print(F(" in "));
      rfid.ShowRubyBlock();
      Serial.print(F(", "));
      if (!WriteData(&rfid))
      {
        Fail();
        return;
      }
      else
      {
        requestType = Get;
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

#endif

#ifdef READER_MODE
      Serial.print(F("  Mode   ==> "));
      Serial.println(reader[index].modeName[reader[index].mode]);

      switch (reader[index].mode)
      {
      case Counting:
      {
        int countMatch = 0;
        for (size_t i = 1; i < NR_OF_RUBY; i++)
        {
          if (!ReadData(&(rfid[i])))
          {
            Fail();
            return;
          }
          else
          {
            if (memcmp(rfid[i].buffer, rfid[i].blockData, 16) == 0)
            {
              Serial.print(F("  Target ==> "));
              rfid[i].ShowRubyName();
              Serial.print(F(" in "));
              rfid[i].ShowRubyBlock();
              Serial.print(F(", "));
              Serial.print(F("Data was read ==> "));
              Serial.println(F("Match "));
              countMatch++;
            }
          }
        }

        Serial.print(F("  Target ==> x"));
        Serial.print(reader[index].countKeys);
        Serial.print(F(" ==> Match x"));
        Serial.print(countMatch);
        Serial.print(F(" ==> "));
        if (countMatch >= reader[index].countKeys)
        {
          Serial.println(F("Unlock"));
          Unlock();
        }
        else
        {
          Serial.println(F("AccessForbidden"));
          AccessForbidden();
        }
        break;
      }

      case Specify:
        Serial.print(F("  Target ==> "));
        reader[index].specificKey->ShowRubyName();
        Serial.print(F(" in "));
        reader[index].specificKey->ShowRubyBlock();
        Serial.print(F(", "));

        if (!ReadData(reader[index].specificKey))
        {
          Fail();
          return;
        }
        else
        {
          Serial.print(F("Data was read ==> "));
          if (memcmp(reader[index].specificKey->buffer, reader[index].specificKey->blockData, 16) == 0)
          {
            Serial.println(F("Match "));
            Unlock();
          }
          else
          {
            Serial.println(F("Incorrect "));
            AccessForbidden();
          }
        }
        break;
      case OR:
      {
        int countMatch = 0;
        for (int i = 0; i < reader[index].countKeys; i++)
        {
          Serial.print(F("  Target ==> "));
          reader[index].passKeys[i]->ShowRubyName();
          Serial.print(F(" in "));
          reader[index].passKeys[i]->ShowRubyBlock();
          Serial.print(F(", "));

          if (!ReadData(reader[index].passKeys[i]))
          {
            Fail();
            return;
          }
          else
          {
            Serial.print(F("Data was read ==> "));
            if (memcmp(reader[index].passKeys[i]->buffer, reader[index].passKeys[i]->blockData, 16) == 0)
            {
              Serial.println(F("Match"));
              countMatch++;
            }
            else
              Serial.println(F("Incorrect"));
          };
        }
        if (countMatch > 0)
          Unlock();
        else
          AccessForbidden();
        break;
      }
      case AND:
      {
        int countMatch = 0;
        for (int i = 0; i < reader[index].countKeys; i++)
        {
          Serial.print(F("  Target ==> "));
          reader[index].passKeys[i]->ShowRubyName();
          Serial.print(F(" in "));
          reader[index].passKeys[i]->ShowRubyBlock();
          Serial.print(F(", "));

          if (!ReadData(reader[index].passKeys[i]))
          {
            Fail();
            return;
          }
          else
          {
            Serial.print(F("Data was read ==> "));
            if (memcmp(reader[index].passKeys[i]->buffer, reader[index].passKeys[i]->blockData, 16) == 0)
            {
              Serial.println(F("Match"));
              countMatch++;
            }
            else
              Serial.println(F("Incorrect"));
          }
        }
        if (countMatch == reader[index].countKeys)
          Unlock();
        else
          AccessForbidden();
        break;
      }
      case Wakaka:
        break;
      case Special:
        break;
      }
#endif

      // 令卡片進入停止狀態
      //   // Halt PICC
      mfrc522[readerSelected].PICC_HaltA();
      //   // Stop encryption on PCD
      mfrc522[readerSelected].PCD_StopCrypto1();
    }
  }
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

void receiveEvent(int howMany)
{
  i2cBufferCnt = 0;
  msg = "";
  while (Wire.available())
  {
    i2cBuffer[i2cBufferCnt] = Wire.read();
    // '\0' as NULL, 255 = unuse index
    if (i2cBuffer[i2cBufferCnt] == '\0' || i2cBuffer[i2cBufferCnt] == 255)
      break;
    msg += (char)i2cBuffer[i2cBufferCnt];
    i2cBufferCnt++;
  }
  hasNewMsg = true;
}
// function that executes whenever data is requested by master
void requestEvent()
{
  if (requestType != None)
  {
// 裝置代號
#ifdef WRITER_MODE
    Wire.write(rfid.badge);
#endif
#ifdef READER_MODE
    for (size_t i = 0; i < 10; i++)
    {
      if (DEVICE_NAME[i] == 0)
        break;
      Wire.write(DEVICE_NAME[i]);
    }
#endif
    readerSelected == 0 ? Wire.write("/0/") : Wire.write("/1/");

    // 特工代號
    for (size_t i = 0; i < 16; i++)
    {
      if (bufferAgentID[i] == 0)
        break;
      Wire.write(bufferAgentID[i]);
    }

    switch (requestType)
    {
    case None:
      break;
    case Cheacking:
      Wire.write("/Cheacking");
      break;
    case Timeout:
      Wire.write("/Timeout");
      break;
    case Cheater:
      Wire.write("/Cheater");
      break;
    case Get:
#ifdef WRITER_MODE
      Wire.write("/Get/");
      for (size_t i = 0; i < 16; i++)
      {
        if (rfid.blockData[i] == 0)
          break;
        Wire.write(rfid.blockData[i]);
      }
#endif
      break;
    case Pass:
      Wire.write("/Pass");
      break;
    case Forbidden:
      Wire.write("/Forbidden");
      break;
    default:
      Wire.write("/");
      break;
    }
    Wire.write("\n");
  }
  requestType = None;
}

bool SetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerSelected].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerSelected].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return false;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Write(blockNum, data, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return false;
  }
  return true;
}

bool GetCardData(byte data[], byte _block[])
{
  byte blockNum = _block[0] * 4 + _block[1]; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _block[0] * 4 + 3;     // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522[readerSelected].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerSelected].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return false;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Read(blockNum, data, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
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
      status = (MFRC522::StatusCode)mfrc522[readerSelected].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerSelected].uid));
      // 若未通過驗證…
      if (status != MFRC522::STATUS_OK)
      {
        // 顯示錯誤訊息
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
        return false;
      }

      // 在指定區塊寫入16位元組資料
      status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Write(blockNum, blockData, 16);
      // 若寫入不成功…
      if (status != MFRC522::STATUS_OK)
      {
        // 顯示錯誤訊息
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
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
  status = (MFRC522::StatusCode)mfrc522[readerSelected].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerSelected].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return false;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Write(blockNum, ruby->blockData, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
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
  status = (MFRC522::StatusCode)mfrc522[readerSelected].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[readerSelected].uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return false;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Read(blockNum, ruby->buffer, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
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
#ifdef WRITER_MODE
  mfrc522[0].PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
#endif
#ifdef READER_MODE
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(100);
  digitalWrite(LED_BUZZER, HIGH);
  delay(500);
  digitalWrite(LED_BUZZER, LOW);
  delay(1000);
  mfrc522[readerSelected].PCD_Init(ssPins[readerSelected], RST_PIN); // Init each MFRC522 card
#endif
}
void Unlock()
{
  requestType = Pass;
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
void AccessForbidden()
{
  requestType = Forbidden;
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