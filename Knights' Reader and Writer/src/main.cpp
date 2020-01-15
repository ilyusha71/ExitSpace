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
/************Copy this code ==> RFID.h***************************************
#define RFID_h
class RFID
{
public:
	RFID(void);
	void Initialize();
	byte ruby;
	byte sector;									// 指定讀寫的「區段」，可能值:0~15，從區段15開始使用
	byte block = 1;										// 指定讀寫的「區塊」，可能值:0~3，區塊3不使用
	byte blockData[16] = {'R', 'u', 'b', 'y', '0'}; // 最多可存入16個字元
													// 若要清除區塊內容，請寫入16個 0
													// byte blockData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	byte buffer[18];								// 暫存讀取區塊內容的陣列，MIFARE_Read()方法要求至少要18位元組空間，來存放16位元組。

private:
};
 ****************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <RFID.h>
void writeBlock(byte _sector, byte _block, byte _blockData[]);
void readBlock(byte _sector, byte _block, byte _blockData[]);
void dump_byte_array(byte *buffer, byte bufferSize);
#include <SPI.h>
#include <MFRC522.h>

#define SLAVE_ADDRESS 1
char incomingByte = 0;
void receiveEvent(int howMany);
void requestEvent();
// #define READER ;
#define WRITER ;
#define NR_OF_RUBY 2
#define RUBY_NUMBER_A 10 // Pass Ruby Number
#define RUBY_NUMBER_B 05 // Pass Ruby Number
byte rubies[] = {RUBY_NUMBER_A, RUBY_NUMBER_B};
#define NR_OF_READERS 2
#define SS_PIN_A 10 // Slave Select Pin A
#define SS_PIN_B 9  // Slave Select Pin B
byte ssPins[] = {SS_PIN_A, SS_PIN_B};
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
    {{60, 209, 110, 133}, "Arduino"},
    {{0xD4, 0xD3, 0xC0, 0x61}, "Raspberry Pi"},
    {{0x15, 0x8, 0xA, 0x53}, "Espruino"}};
byte totalTags = sizeof(tags) / sizeof(RFIDTag);

MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.
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

RFID rfid[NR_OF_RUBY];
#define SIZE_OF_ARRAY(ary) sizeof(ary) / sizeof(*ary)

MFRC522::StatusCode status;

// Flag
enum Work
{
  Ready,
  Failed,
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
  Serial.println(F("/*************************************************/"));
  delay(100);

  // 準備金鑰（用於key A和key B），出廠預設為6組 0xFF。
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

#ifdef READER
  // 初始化讀寫器
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  // 定義本裝置讀寫 RFID 的目標內容
  for (int i = 0; i < NR_OF_RUBY; i++)
  {
    rfid[i].Initialize(rubies[i]);
  }
#endif
#ifdef WRITER
  // 初始化讀寫器
  mfrc522[0].PCD_Init(ssPins[0], RST_PIN); // Init each MFRC522 card
  Serial.print(F("Writer: "));
  mfrc522[0].PCD_DumpVersionToSerial();
  // 定義讀寫 RFID 的目標內容
  rfid[0].Initialize(rubies[0]);
#endif

  Serial.print(F("All Pass is ["));
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

  Serial.println();
  Serial.println(F("2020-01-15_Please scan MIFARE Classic card..."));
  Serial.println(F("/*************************************************/"));

  // pinMode(RST_PIN, OUTPUT);
  // digitalWrite(RST_PIN, LOW);
}

void loop()
{
  work = Ready;
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  {
    // more stable if Init everytimes
    // mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card

    // Look for new cards
    readerSelected = reader;

    if (mfrc522[readerSelected].PICC_IsNewCardPresent() && mfrc522[readerSelected].PICC_ReadCardSerial())
    {
      Serial.print(F("Agent: "));
      byte *id = mfrc522[readerSelected].uid.uidByte; // 取得卡片的UID
      byte idSize = mfrc522[readerSelected].uid.size; // 取得UID的長度
      int indexAgent;
      for (byte i = 0; i < totalTags; i++)
      {
        if (memcmp(tags[i].uid, id, idSize) == 0)
        {
          indexAgent = i;
          Serial.println(tags[i].name); // 顯示標籤的名稱
          break;                        // 退出for迴圈
        }
        else
        {
          Serial.println(F("Arthur"));
        }
      }

#ifdef READER
      for (int indexTarget = 0; indexTarget < 2; indexTarget++)
      {
        Serial.print(F("Target "));
        Serial.print(indexTarget + 1);
        Serial.print(F(": "));
        // 顯示目標名稱
        for (byte j = 0; j < 16; j++)
        {
          if (rfid[indexTarget].blockData[j] == 0)
            break;
          Serial.write(rfid[indexTarget].blockData[j]);
        }

        Serial.print(F(", search ==> "));
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
#endif

#ifdef WRITER
      Serial.print(F("Target: "));
      // 顯示目標名稱
      for (byte j = 0; j < 16; j++)
      {
        if (rfid[0].blockData[j] == 0)
          break;
        Serial.write(rfid[0].blockData[j]);
      }

      Serial.print(F(", search ==> "));
      writeBlock(rfid[0].sector, rfid[0].block, rfid[0].blockData); // 區段編號、區塊編號、存放讀取資料的陣列

      // Master
      // Serial.print(F(" get "));
      // Serial.print(tags[i].name); // 顯示標籤的名稱
      // Serial.print(F(" get "));
      Wire.write("hello\n");

#endif
      // 令卡片進入停止狀態
      //   // Halt PICC
      mfrc522[readerSelected].PICC_HaltA();
      //   // Stop encryption on PCD
      mfrc522[readerSelected].PCD_StopCrypto1();
    }
  }

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
  case OK: // 一長
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
    digitalWrite(LED_BUZZER, LOW);
    delay(1000);
    break;
  case Match: // 一長三短
    digitalWrite(RELAY_LOW, HIGH);
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
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
    delay(3000);
    digitalWrite(RELAY_LOW, LOW);
    break;
  case Incorrect: // 一長兩短
    digitalWrite(LED_BUZZER, HIGH);
    delay(500);
    digitalWrite(LED_BUZZER, LOW);
    delay(100);
    digitalWrite(LED_BUZZER, HIGH);
    delay(300);
    digitalWrite(LED_BUZZER, LOW);
    delay(100);
    digitalWrite(LED_BUZZER, HIGH);
    delay(300);
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
  Wire.write("hello\n"); // respond with message of 6 bytes
                         // as expected by master
}

void writeBlock(byte _sector, byte _block, byte _blockData[])
{
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

  // 在指定區塊寫入16位元組資料
  status = (MFRC522::StatusCode)mfrc522[readerSelected].MIFARE_Write(blockNum, _blockData, 16);
  // 若寫入不成功…
  if (status != MFRC522::STATUS_OK)
  {
    // 顯示錯誤訊息
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522[readerSelected].GetStatusCodeName(status));
    return;
  }

  // 顯示「寫入成功！」
  Serial.println(F("Data was written."));
  if (work != Failed)
    work = OK;
}

void readBlock(byte _sector, byte _block, byte _blockData[])
{
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
