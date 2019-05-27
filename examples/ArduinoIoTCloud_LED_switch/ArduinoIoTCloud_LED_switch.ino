/*
  This sketch demonstrates how to exchange data between your board and the Arduino IoT Cloud.

  Connect a potentiometer (or other analog sensor) to A1 and an LED to Digital Pin 5.
  When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch will work with both WiFi and GSM enabled boards supported by Arduino IoT Cloud.
  By default, settings for WiFi are chosen. If you prefer to use a GSM board take a look at thingProperties.h arduino_secrets.h,
  to make sure you uncomment what's needed and comment incompatible instructions.

  This sketch is compatible with:
   - MKR 1000
   - MKR WIFI 1010
   - MKR GSM 1400
*/
#include "arduino_secrets.h"
#include "thingProperties.h"

void setup() {

  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // wait up to 5 seconds for user to open Serial port
  unsigned long serialBeginTime = millis();
  while (!Serial && (millis() - serialBeginTime > 5000));

  Serial.println("Starting Arduino IoT Cloud Example");

  // initProperties takes care of connecting your sketch variables to the ArduinoIoTCloud object
  initProperties();
  // tell ArduinoIoTCloud to use our WiFi connection
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /*
      The following function allows you to obtain more information
      related to the state of network and IoT Cloud connection and errors
      the higher number the more granular information you'll get.
      The default is 0 (only errors).
      Maximum is 3

      setDebugMessageLevel(3);
  */
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  int potentiometer = analogRead(A1);

}


/*
  this function is called when the "led" property of your Thing changes
*/
void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, led);
}