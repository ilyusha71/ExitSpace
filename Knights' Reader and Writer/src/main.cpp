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
#include <Wire.h>
#include <RFID.h>
void writeBlock(byte _sector, byte _block, byte _blockData[]);
void readBlock(byte _sector, byte _block, byte _blockData[]);
void dump_byte_array(byte *buffer, byte bufferSize);
#include <SPI.h>
#include <MFRC522.h>

// I2C
#define SLAVE_ADDRESS 1
char incomingByte = 0;
void receiveEvent(int howMany);
void requestEvent();
enum RequestType
{
  None,
  Writer,
  Pass,
  At,
} requestType = None;

// #define WRITER ;
#ifdef WRITER
RFID rfid;
#define RUBY_NUMBER 4
MFRC522 mfrc522;
#define SS_PIN 10
#endif

#define READER ;
#ifdef READER
#define NR_OF_RUBY 2
RFID rfid[NR_OF_RUBY];
#define RUBY_NUMBER_A 4  // Pass Ruby Number - Outer
#define RUBY_NUMBER_B 05 // Pass Ruby Number - Inner
byte rubies[] = {RUBY_NUMBER_A, RUBY_NUMBER_B};
#define NR_OF_READERS 2
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
#define SS_PIN_A 10             // Slave Select Pin A
#define SS_PIN_B 9              // Slave Select Pin B
byte ssPins[] = {SS_PIN_A, SS_PIN_B};
#endif

#define RST_PIN 8 // Reset Pin
#define LED_BUZZER 7
#define RELAY_LOW 6

// 定義RFID卡片標籤
typedef struct
{ // 宣告自訂的結構資料類型
  byte uid[4];
  const char *name;
} RFIDTag;

RFIDTag tags[] = { // 初始化結構資料
    {{0xFF, 0xFF, 0xFF, 0xFF}, "Unknown"},
    {{0xF9, 0x3A, 0x3D, 0x0D}, "KGB-001"},
    {{0x1A, 0x81, 0x4E, 0x84}, "KGB-002"},
    {{0xF5, 0xc4, 0x53, 0x84}, "KGB-003"},
    {{0x3D, 0x85, 0xEA, 0x75}, "KGB-004"},
    {{0x15, 0x76, 0x76, 0x63}, "KGB-005"},
    {{0x97, 0x1E, 0xE0, 0xBE}, "KGB-006"},
    {{0x4A, 0x5E, 0x28, 0x0C}, "KGB-007"},
    {{0x96, 0xDC, 0x4D, 0x59}, "KGB-008"},
    {{0xE7, 0x6F, 0xF3, 0x38}, "KGB-009"},
    {{0xA4, 0xDB, 0xB0, 0xD5}, "KGB-010"},
    {{0x10, 0x6F, 0x46, 0xD5}, "MI6-201"},
    {{0x6A, 0x9D, 0xFB, 0xB8}, "MI6-202"},
    {{0x46, 0x4A, 0x19, 0x00}, "MI6-203"},
    {{0xF1, 0xE2, 0xA6, 0xD5}, "MI6-204"},
    {{0x46, 0x8A, 0x30, 0x00}, "MI6-205"},
    {{0x65, 0xAA, 0xA6, 0xD5}, "MI6-206"},
    {{0x2A, 0x0A, 0xCA, 0x10}, "MI6-207"},
    {{0xF7, 0xE2, 0x73, 0x62}, "MI6-208"},
    {{0xF7, 0x83, 0x73, 0x62}, "MI6-209"},
    {{0xD7, 0x50, 0x7C, 0x62}, "MI6-210"},
    {{0xF9, 0x3A, 0x3D, 0x0D}, "Example"}};
byte totalTags = sizeof(tags) / sizeof(RFIDTag);
int indexAgent = 0;

// Switcher
unsigned long timer;
unsigned long counter = 0;
int readerSelected;

MFRC522::MIFARE_Key key;           // 儲存金鑰
byte wakakaKey[16] = "Wakaka Key"; // 最多可存入16個字元
// 若要清除區塊內容，請寫入16個 0
//byte blockData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// 暫存讀取區塊內容的陣列，MIFARE_Read()方法要求至少要18位元組空間，來存放16位元組。
byte buffer[18];

#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

MFRC522::StatusCode status;

// Flag
enum Work
{
  Ready,
  Failed,
  Written,
  OK,
  Match,
  Incorrect,
};
Work work = Ready;

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

#ifdef WRITER
  // 定義 RFID 寫入區域與內容
  Serial.print(F(" * Stage 1, "));
  rfid.Initialize(RUBY_NUMBER);
  Serial.print(F(" Badge Writer in [Sector "));
  Serial.print(rfid.sector);
  Serial.print(F("] [Block "));
  Serial.print(rfid.block);
  Serial.println(F("]"));
  // 初始化 MFRC522
  Serial.print(F(" * Writer Status ==> "));
  mfrc522.PCD_Init(SS_PIN, RST_PIN); // Init each MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();
#endif

#ifdef READER

  Serial.println(F(" * Stage 2"));
  for (int i = 0; i < NR_OF_RUBY; i++)
  {
    // 定義 RFID 讀取區域與內容
    Serial.print(F(" ********* "));
    rfid[i].Initialize(rubies[i]);
    Serial.print(F(" Badge Reader in [Sector "));
    Serial.print(rfid[i].sector);
    Serial.print(F("] [Block "));
    Serial.print(rfid[i].block);
    Serial.println(F("]"));
    // 初始化 MFRC522
    Serial.print(F(" * Reader Status ==> "));
    mfrc522[i].PCD_Init(ssPins[i], RST_PIN); // Init each MFRC522 card
    mfrc522[i].PCD_DumpVersionToSerial();
  }
#endif

  // 萬用鑰匙
  Serial.print(F(" * All Pass is ["));
  for (byte i = 0; i < 16; i++)
  {
    if (wakakaKey[i] == 0)
      break;
    Serial.write(wakakaKey[i]);
  }
  Serial.println(F("]"));

  pinMode(LED_BUZZER, OUTPUT);
  pinMode(RELAY_LOW, OUTPUT);
  digitalWrite(LED_BUZZER, LOW);
  digitalWrite(RELAY_LOW, LOW);

  Serial.println(F(" *"));
  Serial.println(F(" * 2020-01-21_Please scan MIFARE Classic card..."));
  Serial.println(F(" ****************************************************************************/"));

  // pinMode(RST_PIN, OUTPUT);
  // digitalWrite(RST_PIN, LOW);
}

void loop()
{
  work = Ready;

#ifdef READER
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    // more stable if Init everytimes
    // mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card

    // Look for new cards
    readerSelected = reader;

    if (mfrc522[readerSelected].PICC_IsNewCardPresent() && mfrc522[readerSelected].PICC_ReadCardSerial())
    {
      byte *id = mfrc522[readerSelected].uid.uidByte; // 取得卡片的UID
      byte idSize = mfrc522[readerSelected].uid.size; // 取得UID的長度
      indexAgent = 0;
      for (byte i = 1; i < totalTags; i++)
      {
        if (memcmp(tags[i].uid, id, idSize) == 0)
        {
          indexAgent = i;
          break;
        }
      }
      Serial.print(F("Agent: ["));
      Serial.print(tags[indexAgent].name);
      Serial.print(F("] ==> id: "));
      for (byte i = 0; i < idSize; i++)
      {
        Serial.print(id[i] < 16 ? " 0" : " "); // 以16進位顯示UID值
        Serial.print(id[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();

      // 製作陣列使用
      // Serial.print(F("{{"));
      // for (byte i = 0; i < idSize; i++)
      // {
      //   Serial.print(F("0x"));
      //   Serial.print(id[i] < 0x16 ? "0" : ""); // 以16進位顯示UID值
      //   Serial.print(id[i], HEX);
      //   if (i != (idSize - 1))
      //     Serial.print(F(", "));
      // }
      // Serial.println(F("}, ABCDEF"));

      for (int indexTarget = 0; indexTarget < 2; indexTarget++)
      {
        Serial.print(F("Target "));
        Serial.print(indexTarget + 1);
        Serial.print(F(": ["));
        // 顯示目標名稱
        for (byte j = 0; j < 16; j++)
        {
          if (rfid[indexTarget].blockData[j] == 0)
            break;
          Serial.write(rfid[indexTarget].blockData[j]);
        }

        Serial.print(F("], search ==> "));
        readBlock(rfid[indexTarget].sector, rfid[indexTarget].block, rfid[indexTarget].buffer); // 區段編號、區塊編號、存放讀取資料的陣列

        if (work == Failed)
          break;
        // 秀出寫入的資料與記憶體位置
        for (byte i = 0; i < 16; i++)
        {
          if (rfid[indexTarget].buffer[i] == 0)
            break;
          Serial.write(rfid[indexTarget].buffer[i]);
        }

        Serial.print(F(" ==> "));
        if (memcmp(rfid[indexTarget].buffer, rfid[indexTarget].blockData, 16) == 0 ||
            memcmp(rfid[indexTarget].buffer, wakakaKey, 16) == 0) // 判斷用
        {
          Serial.println(F("Match"));
          if (work == OK)
            work = Match;
        }
        else
        {
          Serial.println(F("Incorrect"));
          if (work == OK)
            work = Incorrect;
        }
      }

      // 令卡片進入停止狀態
      //   // Halt PICC
      mfrc522[readerSelected].PICC_HaltA();
      //   // Stop encryption on PCD
      mfrc522[readerSelected].PCD_StopCrypto1();
    }
  }
#endif

#ifdef WRITER
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    byte *id = mfrc522.uid.uidByte; // 取得卡片的UID
    byte idSize = mfrc522.uid.size; // 取得UID的長度
    indexAgent = 0;
    for (byte i = 1; i < totalTags; i++)
    {
      if (memcmp(tags[i].uid, id, idSize) == 0)
      {
        indexAgent = i;
        break;
      }
    }
    Serial.print(F("Agent: ["));
    Serial.print(tags[indexAgent].name);
    Serial.print(F("] ==> id: "));
    for (byte i = 0; i < idSize; i++)
    {
      Serial.print(id[i] < 16 ? " 0" : " "); // 以16進位顯示UID值
      Serial.print(id[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println();
    Serial.print(F("Target: "));
    // 顯示目標名稱
    for (byte j = 0; j < 16; j++)
    {
      if (rfid.blockData[j] == 0)
        break;
      Serial.write(rfid.blockData[j]);
    }
    Serial.print(F(" ["));
    Serial.print(rfid.badge);
    Serial.print(F("], search ==> "));
    writeBlock(rfid.sector, rfid.block, rfid.blockData); // 區段編號、區塊編號、存放讀取資料的陣列
    // 令卡片進入停止狀態
    //   // Halt PICC
    mfrc522.PICC_HaltA();
    //   // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  }
#endif

  switch (work)
  {
  case Failed: // 兩長
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
    digitalWrite(LED_BUZZER, LOW);
    delay(100);
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
    digitalWrite(LED_BUZZER, LOW);
    delay(1000);
    break;
  case Written: // 一長
    requestType = Writer;
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
    digitalWrite(LED_BUZZER, LOW);
    delay(1000);
    break;
  case Match: // 一長三短
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
    delay(3000);
    digitalWrite(RELAY_LOW, LOW);
    break;
  case Incorrect: // 一長兩短
    requestType = At;
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
    break;
  default:
    break;
  }
}

void receiveEvent(int howMany)
{
  while (Wire.available())
  {
    // receive one byte from Master
    incomingByte = Wire.read();
    Serial.print(incomingByte);
  }
}
// function that executes whenever data is requested by master
void requestEvent()
{
  switch (requestType)
  {
  case Writer:
#ifdef WRITER
    Wire.write("[");
    Wire.write(tags[indexAgent].name);
    Wire.write("] have got [");
    for (byte j = 0; j < 16; j++)
    {
      if (rfid.blockData[j] == 0)
        break;
      Wire.write(rfid.blockData[j]);
    }
    Wire.write("]\n");
#endif
    requestType = None;
    break;
  case Pass:
#ifdef READER
    Wire.write("[");
    Wire.write(tags[indexAgent].name);
    Wire.write("] have Pass ["); // 讀取器尚未命名
    for (byte j = 0; j < 16; j++)
    {
      if (rfid[0].blockData[j] == 0)
        break;
      Wire.write(rfid[0].blockData[j]);
    }
    Wire.write("]\n");
#endif
    requestType = None;
    break;
  case At:
    Wire.write("[");
    Wire.write(tags[indexAgent].name);
    Wire.write("] is at ["); // 讀取器尚未命名

    for (byte j = 0; j < 16; j++)
    {
#ifdef WRITER
      if (rfid.blockData[j] == 0)
        break;
      Wire.write(rfid.blockData[j]);
#endif
#ifdef READER
      if (rfid[0].blockData[j] == 0)
        break;
      Wire.write(rfid[0].blockData[j]);
#endif
    }

    Wire.write("]\n");
    requestType = None;
    break;
  default:
    break;
  }
}

void writeBlock(byte _sector, byte _block, byte _blockData[])
{
#ifdef WRITER
  if (_sector < 0 || _sector > 15 || _block < 0 || _block > 3)
  {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return;
  }

  if (_sector == 0 && _block == 0)
  {
    // 顯示「第一個區塊只能讀取」，然後結束函式。
    Serial.println(F("First block is read-only."));
    return;
  }

  // iK.顯示寫入的位置
  Serial.print(F("Sector "));
  Serial.print(_sector);
  Serial.print(F(", Block "));
  Serial.print(_block);
  Serial.print(F(", "));

  byte blockNum = _sector * 4 + _block; // 計算區塊的實際編號（0~63）
  byte trailerBlock = _sector * 4 + 3;  // 控制區塊編號

  // 驗證金鑰
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  // 若未通過驗證…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    work = Failed;
    return;
  }

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockNum, _blockData, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 顯示「寫入成功！」
  Serial.println(F("Data was written."));
  if (work != Failed)
    work = Written;
#endif
}

void readBlock(byte _sector, byte _block, byte _blockData[])
{
#ifdef READER
  if (_sector < 0 || _sector > 15 || _block < 0 || _block > 3)
  {
    // 顯示「區段或區塊碼錯誤」，然後結束函式。
    Serial.println(F("Wrong sector or block number."));
    return;
  }
  // iK.顯示讀取的位置
  Serial.print(F("Sector "));
  Serial.print(_sector);
  Serial.print(F(", Block "));
  Serial.print(_block);
  Serial.print(F(", "));

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
    work = Failed;
    return;
  }

  byte buffersize = 18;
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Read(blockNum, _blockData, &buffersize);

  // 若讀取不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_read() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return;
  }

  // 顯示「讀取成功！」
  Serial.print(F("Data was read ==> "));
  if (work != Failed && work != Match)
    work = OK;
#endif
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
