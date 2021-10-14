/*
  This sketch demonstrates how to exchange data between your board and the Arduino IoT Cloud.

  * Connect a potentiometer (or other analog sensor) to A0.
  * When the potentiometer (or sensor) value changes the data is sent to the Cloud.
  * When you flip the switch in the Cloud dashboard the onboard LED lights gets turned ON or OFF.

  IMPORTANT:
  This sketch works with WiFi, GSM, NB and Lora enabled boards supported by Arduino IoT Cloud.
  On a LoRa board, if it is configuered as a class A device (default and preferred option), values from Cloud dashboard are received
  only after a value is sent to Cloud.

  This sketch is compatible with:
   - MKR 1000
   - MKR WIFI 1010
   - MKR GSM 1400
   - MKR NB 1500
   - MKR WAN 1300/1310
   - Nano 33 IoT
   - ESP 8266
*/

#include "arduino_secrets.h"
#include "thingProperties.h"

#if defined(ESP32)
static int const LED_BUILTIN = 2;
#endif

/*
// Can use it as soon as https://github.com/arduino-libraries/Arduino_ConnectionHandler/pull/63 gets merged

ConnectionHandler& get_default_connection_handler() {
  char ssid[MAX_LEN];
  char password[MAX_LEN];
  retrieveCredentialsFromFile("/fs/credential.txt", ssid, password);
  static WiFiConnectionHandlerDynamic ArduinoIoTPreferredConnection(ssid, password);
  return ArduinoIoTPreferredConnection;
}
*/

bool always_deny() {
  return false;
}

bool always_allow() {
  return true;
}

static bool ask_user_via_serial_first_run = true;
bool ask_user_via_serial() {
  if (ask_user_via_serial_first_run) {
    Serial.println("Apply OTA? y / [n]");
    ask_user_via_serial_first_run = false;
  }
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'y' || c == 'Y') {
      return true;  
    }
  }
  return false;
}

void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(9600);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }

  /* Configure LED pin as an output */
  pinMode(LED_BUILTIN, OUTPUT);

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(get_default_connection_handler());

  ArduinoCloud.onOTARequestCb(always_deny);

  setDebugMessageLevel(DBG_INFO);
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
