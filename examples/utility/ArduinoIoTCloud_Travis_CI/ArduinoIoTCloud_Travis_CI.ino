/*
   This sketch is used in combination with Travis CI to check if
   unintentional breaking changes are made to the user facing
   Arduino IoT Cloud API.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB, Ethernet and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configured as a class A device (default and preferred option), values from Cloud dashboard are received
  only after a value is sent to Cloud.

  The full list of compatible boards can be found here:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include "arduino_secrets.h"
#include "thingProperties.h"

void setup() {

  Serial.begin(9600);
  unsigned long serialBeginTime = millis();
  while (!Serial && (millis() - serialBeginTime > 5000));

  Serial.println("Starting Arduino IoT Cloud Example");

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

}

void onBoolPropertyChange() {
  Serial.println("'onBoolPropertyChange'");
}

void onIntPropertyChange() {
  Serial.println("'onIntPropertyChange'");
}

void onFloatPropertyChange() {
  Serial.println("'onFloatPropertyChange'");
}

void onStringPropertyChange() {
  Serial.println("'onStringPropertyChange'");
}
