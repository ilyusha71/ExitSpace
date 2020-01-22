#include <Arduino.h>

// master_sender.ino
// Refer to the "slave_receiver" example for use with this
#include <Wire.h>

const int SLAVE_ADDRESS = 1;
char incomingByte = 0;
byte messages[32];
void setup()
{
  Wire.begin(); // join I2C bus as a Master

  Serial.begin(9600);
  Serial.println("Type something to send:");
}

void loop()
{
  Wire.requestFrom(SLAVE_ADDRESS, 100); // request 6 bytes from slave

  int count = 0;
  while (Wire.available()) // slave may send less than requested
  {
    incomingByte = Wire.read(); // receive a byte
    messages[count] = incomingByte;
    count++;
    if (incomingByte == NULL)
      break;
    Serial.print(incomingByte); // print the character
    if (incomingByte == '\n')
    {
      Serial.println(count);
      for (int j = 0; j < 32; j++)
      {
        if (messages[j] == NULL)
          break;
        Serial.write(messages[j]);
      }
      return;
    }
  }
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