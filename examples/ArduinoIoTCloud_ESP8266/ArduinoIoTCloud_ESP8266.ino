/* 
 * Minimal demo example how to connect with the
 * Arduino IoT Cloud and a ESP8266 based WiFi board.
 */

#include "arduino_secrets.h"
#include "thingProperties.h"

#define ARDUINO_CLOUD_DEVICE_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" /* Entry "Board ID" when selecting board within Arduino Create */
#define ARDUINO_CLOUD_DEVICE_PASS "my-password" /* Password set via IoT API */

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  
  /* Wait up to 5 seconds for user to open serial port */
  unsigned long serialBeginTime = millis();
  while (!Serial && (millis() - serialBeginTime > 5000));

  initProperties();
  
  setDebugMessageLevel(DBG_INFO);
  
  ArduinoCloud.begin(ArduinoIoTPreferredConnection, ARDUINO_CLOUD_DEVICE_ID, ARDUINO_CLOUD_DEVICE_PASS);
}

void loop() {
  ArduinoCloud.update();
}


void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, !led);
}
