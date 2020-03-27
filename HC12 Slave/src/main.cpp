#include <Arduino.h>

/*    Arduino Long Range Wireless Communication using HC-12
                      Example 01
   by Dejan Nedelkovski, www.HowToMechatronics.com
*/
#include <SoftwareSerial.h>
SoftwareSerial HC12(2, 3); // HC-12 TX Pin, HC-12 RX Pin

String getValue(String data, char separator, int index);
void setup()
{
  Serial.begin(9600);           // Serial port to computer
  HC12.begin(9600);             // Serial port to HC12
  Serial.println(F("slave02")); // Send the data to Serial monitor
}
bool recive = false;
bool ready = false;

uint8_t recBuffer;
String mmm = "";
unsigned long timer;
void loop()
{
  // HC12.println("1212");
  //  HC12.println("sdsddsd");
  while (HC12.available())
  { // If HC-12 has data
    recBuffer = HC12.read();
    if (recBuffer == '\0' || recBuffer == 255)
      break;
    mmm += (char)recBuffer;
    //    Serial.write();      // Send the data to Serial monitor
    if (recBuffer == 10)
      recive = true;
  }

  if (recive)
  {
    recive = false;
 
    Serial.print(mmm);
    String msgType = getValue(mmm, '/', 0);
    mmm = "";
    if (msgType == "S")
    {
      ready = true;
      timer = millis() + 750;
    }
  }
  if (millis() > timer && ready)
  {
    // Serial.println("OK");
    ready = false;
    HC12.println("Slave    04 OK");
  }
  if (millis() < timer)
  {
    // Serial.println(timer-millis());
  }
  
  // while (Serial.available())
  // {                            // If Serial monitor has data
  //   HC12.write(Serial.read()); // Send that data to HC-12
  // }
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