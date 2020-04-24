const int Input_A = 3;
const int Input_B = 5;

// Input
int TriggerValue_A;
int TriggerValue_B;
int durationMax = 1000; // 超過1000，Arduino會出錯
int durationLimit = 10;

// Unity
char msgSerial;

// Checking
String commandUnity;
bool hasCommand;
unsigned long lateTime;
const int checkInterval = 3000; // 與Unity溝通檢查間隔(ms)

void setup() 
{
  Serial.begin(9600);
  pinMode(Input_A,INPUT_PULLUP);
  pinMode(Input_B,INPUT_PULLUP);
  Serial.println("Arduino Testing");
}

void loop() 
{
      
      // 讀取序列阜（包含Unity的字元訊息）
      msgSerial = Serial.read();

      // Arduino-Unity連線Callback
      if(msgSerial == 'R')
          Serial.println("ArduinoCallback/");
      
      hasCommand = false;
      commandUnity = "Wakaka/";

      // Input
      TriggerValue_A = Digital2Value(TriggerValue_A, digitalRead(Input_A));
      TriggerValue_B = Digital2Value(TriggerValue_B, digitalRead(Input_B));
      
      if(TriggerValue_A >= durationLimit)
      {
          hasCommand =true;
          commandUnity += "TriggerA";
          commandUnity += "/";
      }
      else
          commandUnity += "NoA/";
      
      if(TriggerValue_B >= durationLimit)
      {
          hasCommand =true;
          commandUnity += "TriggerB";
          commandUnity += "/";
      }
      else
          commandUnity += "NoB/";
          
      /* 溝通檢查 */
      if((millis()-lateTime)>checkInterval || hasCommand)
      {
          Serial.println(commandUnity);
          lateTime = millis();
      }
}

int Digital2Value(int value, int digital)
{
    digital == 0 ? value++ : value = 0;
    return value;
}
