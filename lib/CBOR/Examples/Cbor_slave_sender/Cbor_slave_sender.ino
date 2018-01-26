/*

Arduino Sketch to show how Send Encode Cbor package and sending via I2c as Slave requester
This sketch must to load in the Slave Arduino

Author: Juanjo Tara 
email:  j.tara@arduino.cc
date: 	24/04/2015
*/




#include "CborEncoder.h"
#include <Wire.h>


void setup() {
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event
  Serial.begin(9600);
  // writting();
}

void loop() {
  
}


// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{

   //get length and data of cbor package        
  unsigned char *datapkg = output.getData();
  int datalength = output.getSize();
  
  Serial.print("datalength:");
  Serial.print(datalength);
  
  Wire.write(*datapkg); // respond with message 

}
