#include <Arduino.h>

// master_sender.ino
// Refer to the "slave_receiver" example for use with this
#include <Wire.h>

const int SLAVE_ADDRESS = 1;
char incomingByte = 0;

void setup() {  
  Wire.begin();         // join I2C bus as a Master
  
  Serial.begin(9600);
  Serial.println("Type something to send:");
}

void loop() {
  Wire.requestFrom(SLAVE_ADDRESS, 6); // request 6 bytes from slave
  
  while (Wire.available())            // slave may send less than requested
  {
    incomingByte = Wire.read();       // receive a byte 
    Serial.print(incomingByte);        // print the character
  }  
  
  delay(1000);
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