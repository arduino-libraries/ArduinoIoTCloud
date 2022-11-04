/*
  This sketch demonstrates how to subscribe to IoT Cloud events and perform actions
  The available events are

  CONNECT     : Board successfully connects to IoT Cloud
  SYNC        : Data is successfully synced between Board and IoT Cloud
  DISCONNECT  : Board has lost connection to IoT Cloud

  You don't need any specific Properties to be created in order to demonstrate these functionalities.
  Simply create a new Thing and give it 1 arbitrary Property.
  Remember that the Thing ID needs to be configured in thingProperties.h 
  These events can be very useful in particular cases, for instance to disable a peripheral
  or a connected sensor/actuator when no data connection is available, as well as to perform
  specific operations on connection or right after properties values are synchronised.

  To subscribe to an event you can use the `addCallback` method and specify
  which event will trigger which custom function.
  One function per event can be assigned.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configured as a class A device (default and preferred option), values from Cloud dashboard are received
  only after a value is sent to Cloud.

  This sketch is compatible with:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include "arduino_secrets.h"
#include "thingProperties.h"

void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
      Invoking `addCallback` on the ArduinoCloud object allows you to subscribe
      to any of the available events and decide which functions to call when they are fired.

      The functions `doThisOnConnect`, `doThisOnSync`, `doThisOnDisconnect`
      are custom functions and can be named to your likings and for this example
      they are defined/implemented at the bottom of the Sketch
  */
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::SYNC, doThisOnSync);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);

  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
}

void doThisOnConnect(){
  /* add your custom code here */
  Serial.println("Board successfully connected to Arduino IoT Cloud");
}
void doThisOnSync(){
  /* add your custom code here */
  Serial.println("Thing Properties synchronised");
}
void doThisOnDisconnect(){
  /* add your custom code here */
  Serial.println("Board disconnected from Arduino IoT Cloud");
}