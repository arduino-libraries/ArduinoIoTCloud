/*

Arduino Sketch to show how Send Encode Cbor package via Serial port

Author: Juanjo Tara 
email:  j.tara@arduino.cc
date:   24/04/2015
*/


#include "CborEncoder.h"
#include "SerialBuffer.h"

#define SERIAL_PORT Serial

#define BUFFER_SIZE 32
unsigned char buffer[BUFFER_SIZE];

// declare the serial buffer
SerialBuffer serialBuffer;


void setup() {
  
  // set up the buffer storage and maximum size
  serialBuffer.buffer = buffer;
  serialBuffer.bufferSize = BUFFER_SIZE;

  // reset the buffer
  serialBuffer.reset();
  Serial.begin(115200);
  randomSeed(analogRead(0));

}

void loop() {

  testSerialPort();

  int maxBytes = SERIAL_PORT.available();
  while (maxBytes--) {

    byte inputByte = SERIAL_PORT.read();

    // present the input byte to the serial buffer for decoding
    // whenever receive() returns >= 0, there's a complete message
    // in the buffer ready for processing at offset zero.
    // (return value is message length)
    int bufferStatus = serialBuffer.receive(inputByte);

    if (bufferStatus >= 0) {

      // handle message
      // ...
      // ...

    }
  }
  delay(100);
  
}

void testSerialPort() {
    CborStaticOutput output(32);
    CborWriter writer(output);
    //Write a Cbor Package with a number and String  
    int randNumb = random(15);
    for(int i = 0; i < randNumb; i++) {
      int numb = random(300);
      writer.writeInt(numb);
    }
    //writer.writeInt(0x7F);
    //writer.writeInt(0x7E);
    //writer.writeInt(0x7D);
    //writer.writeString("Hello David");
    //writer.writeInt(321);

    //get length and data of cbor package        
    unsigned char *datapkg = output.getData();
    int length = output.getSize();
    serialBuffer.startMessage();
    for(int i = 0; i < length; i++) {
      serialBuffer.write(datapkg[i]);
    }
    serialBuffer.endMessage();
    //print in Serial port the Data length and Cbor in binary
    //Serial.print("datalength:");
    //Serial.print(output.getSize());
    //Serial.println(output.getSize());
    Serial.write(serialBuffer.buffer, serialBuffer.messageLength());
}