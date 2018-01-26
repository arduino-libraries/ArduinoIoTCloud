/*

Arduino Sketch to show how encode and Decode Cbor package

Author: Juanjo Tara 
email:  j.tara@arduino.cc
date:	24/04/2015
*/




#include "CborEncoder.h"
#include "CborDecoder.h"

//String to save the cbor data
String dt = "";
String valuetoInt = "";	
unsigned int sizeee;
int valuein;


void setup() {

  Serial.begin(9600);
  //Serial.print("hola");
  test1();
}

void loop() {


}



void test1() {

	//Create object and Writer
    	CborStaticOutput output(32);
    	CborWriter writer(output);

    	//Write a Cbor Package with a number and String 
   	 writer.writeInt(124);
	writer.writeString("I");

    	sizeee = output.getSize();
    	Serial.print("datalength:");
    	Serial.println(sizeee);

    	delay(1000);

	//Receiver for the Cbor input
    	CborInput input(output.getData(), output.getSize());
	CborDebugListener listener;
	CborReader reader(input);
	reader.SetListener(listener);
	//Save all the cbor into a String divived by commas 
	reader.GetCborData(dt);
   

	valuetoInt = dt.substring(0, dt.indexOf(','));
	valuein = valuetoInt.toInt();
  
	Serial.print(valuetoInt.toInt());

}
