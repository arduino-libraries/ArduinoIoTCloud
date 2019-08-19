#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#define THING_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" /* "Thing ID" when selecting thing within Arduino Create */
#define DEVICE_LOGIN_NAME "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"

void onLedChange();

bool led;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.setDeviceLoginName(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
  ArduinoCloud.addProperty(led, READWRITE, ON_CHANGE, onLedChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
