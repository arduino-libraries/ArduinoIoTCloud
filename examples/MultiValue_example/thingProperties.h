#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// Set the Thing Id value
const char THING_ID[] = "";

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onSwitchButtonChange();
void onColorChange();

bool switchButton;
CloudLocation location;
CloudColor color;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(switchButton, READWRITE, ON_CHANGE, onSwitchButtonChange);
  ArduinoCloud.addProperty(location, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(color, READWRITE, ON_CHANGE, onColorChange);
}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
