/*
   This sketch is used in combination with Travis CI to check if
   unintentional breaking changes are made to the used facing
   Arduino IoT Cloud API.

   This sketch is compatible with:
     - MKR 1000
     - MKR WIFI 1010
     - MKR GSM 1400
     - MKR WAN 1300/1310
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
