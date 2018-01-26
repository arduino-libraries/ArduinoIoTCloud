/*

Arduino Sketch to show how decode a Cbor package received from I2c
This sketch must to load in the master Arduino

Author: Juanjo Tara 
email:  j.tara@arduino.cc
date:	24/04/2015
*/


#include <Wire.h>
#include <CborDecoder.h>


void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

}

void loop()
{
  Wire.requestFrom(2, 4);    // request 6 bytes from slave device #2

  while (Wire.available())   // slave may send less than requested
  {
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }

  CborInput input(data, size);
  CborReader reader(input);
  CborExampleListener listener;
  reader.SetListener(listener);
  reader.Run();
  delay(500);
}
