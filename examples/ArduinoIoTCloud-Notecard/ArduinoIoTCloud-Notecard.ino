/*
  This sketch demonstrates how to exchange data between your board and the
  Arduino IoT Cloud, while using the Notecard for wireless communication.

  * Connect a potentiometer (or other analog sensor) to A0.
  * When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  * When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch works with any Wi-Fi, Cellular, LoRa or Satellite enabled Notecard.

  The full list of compatible boards can be found here:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include <Notecard.h>
#include "thingProperties.h"

#if !defined(LED_BUILTIN) && !defined(ARDUINO_NANO_ESP32)
static int const LED_BUILTIN = 2;
#endif

/*
 * Choose an interrupt capable pin to reduce polling and improve
 * the overall responsiveness of the ArduinoIoTCloud library
 */
// #define ATTN_PIN 9

void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime <= 5000); ) { }

  /* Set the debug message level:
   * - DBG_ERROR: Only show error messages
   * - DBG_WARNING: Show warning and error messages
   * - DBG_INFO: Show info, warning, and error messages
   * - DBG_DEBUG: Show debug, info, warning, and error messages
   * - DBG_VERBOSE: Show all messages
   */
  setDebugMessageLevel(DBG_INFO);

  /* Configure LED pin as an output */
  pinMode(LED_BUILTIN, OUTPUT);

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
#ifndef ATTN_PIN
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  ArduinoCloud.setNotecardPollingInterval(3000);  // default: 1000ms, min: 250ms
#else
  ArduinoCloud.begin(ArduinoIoTPreferredConnection, ATTN_PIN);
#endif

  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  potentiometer = analogRead(A0);
  seconds = millis() / 1000;
}

/*
 * 'onLedChange' is called when the "led" property of your Thing changes
 */
void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, led);
}
