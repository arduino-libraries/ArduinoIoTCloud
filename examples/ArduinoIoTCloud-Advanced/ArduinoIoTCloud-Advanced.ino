/*
  This sketch demonstrates how to use more complex cloud data types such as a colour or coordinates.

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
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(DBG_INFO);
  ArduinoCloud.printDebugInfo();
}

float latMov = 45.5058224, lonMov = 9.1628673;
float latArd = 45.0502078, lonArd = 7.6674765;

float hueRed = 0.0, satRed = 100.0, briRed = 100.0;
float hueGreen = 80.0, satGreen = 100.0, briGreen = 100.0;

void loop() {
  ArduinoCloud.update();
}

void onSwitchButtonChange() {
  if (switchButton)
  {
    location = Location(latMov, lonMov);
    color    = Color(hueRed, satRed, briRed);
  }
  else
  {
    location = Location(latArd, lonArd);
    color    = Color(hueGreen, satGreen, briGreen);
  }
}

void onColorChange() {
  Serial.print("Hue = ");
  Serial.println(color.getValue().hue);
  Serial.print("Sat = ");
  Serial.println(color.getValue().sat);
  Serial.print("Bri = ");
  Serial.println(color.getValue().bri);
}
