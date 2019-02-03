#include <WiFiConnectionManager.h>

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(
  SECRET_SSID, SECRET_PASS);

void onLedSwitchChange();
void onIntValueChange();

bool ledSwitch;
int intValue;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(ledSwitch, READWRITE, ON_CHANGE, onLedSwitchChange);
  ArduinoCloud.addProperty(intValue, READWRITE, ON_CHANGE, onIntValueChange);
}
