#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#define THING_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" /* "Thing ID" when selecting thing within Arduino Create */
#define DEVICE_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" /* Entry "Board ID" when selecting board within Arduino Create */

void onLedChange();

bool led;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.setDeviceId(DEVICE_ID);
  ArduinoCloud.setPassword(SECRET_DEVICE_PASS);
  ArduinoCloud.addProperty(led, READWRITE, ON_CHANGE, onLedChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
