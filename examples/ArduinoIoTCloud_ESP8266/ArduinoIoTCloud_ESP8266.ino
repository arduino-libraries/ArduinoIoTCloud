/*
   Minimal demo example how to connect with the
   Arduino IoT Cloud and a ESP8266 based WiFi board.
*/

#include "arduino_secrets.h"
#include "thingProperties.h"

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);

  /* Wait up to 5 seconds for user to open serial port */
  unsigned long serialBeginTime = millis();
  while (!Serial && (millis() - serialBeginTime > 5000));

  initProperties();

  setDebugMessageLevel(DBG_INFO);

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}

void loop() {
  ArduinoCloud.update();
}


void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, !led);
}
