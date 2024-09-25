/*
  This sketch demonstrates how to optimize OTA in case of complex loop().

  * Connect a potentiometer (or other analog sensor) to A0.
  * When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  * When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB, Ethernet and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configured as a class A device (default and preferred option),
  values from Cloud dashboard are received only after a value is sent to Cloud.

  The full list of compatible boards can be found here:
   - https://github.com/arduino-libraries/ArduinoIoTCloud#what
*/

#include "thingProperties.h"

#if !defined(LED_BUILTIN) && !defined(ARDUINO_NANO_ESP32)
static int const LED_BUILTIN = 2;
#endif

bool block_for_ota { false };
bool ota_started { false };

bool onOTARequestCallback() {
  block_for_ota = true;
  ota_started = true;
  return true;
}

constexpr unsigned long printInterval { 1000 };
unsigned long printNow { printInterval };

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
  setDebugMessageLevel(DBG_VERBOSE);

  /* Configure LED pin as an output */
  pinMode(LED_BUILTIN, OUTPUT);

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  /* Setup OTA callback */
  ArduinoCloud.onOTARequestCb(onOTARequestCallback);

  ArduinoCloud.printDebugInfo();
}

void loop() {
  // When OTA is available, stay there until it completes.
  // The rest of the loop() does not run and the sketch
  // restarts automatically at the end of the OTA process.
  while (block_for_ota) {
    ArduinoCloud.update();
    if (ota_started) {
      Serial.print("Waiting for OTA to finish...");
      ota_started = false;
    }
    if (millis() > printNow) {
      Serial.print(".");
      printNow = millis() + printInterval;
    }    
  }

  ArduinoCloud.update();
  potentiometer = analogRead(A0);
  seconds = millis() / 1000;

  if (millis() > printNow) {
    Serial.println(millis());
    printNow = millis() + printInterval;
  }
}

/*
 * 'onLedChange' is called when the "led" property of your Thing changes
 */
void onLedChange() {
  Serial.print("LED set to ");
  Serial.println(led);
  digitalWrite(LED_BUILTIN, led);
}
