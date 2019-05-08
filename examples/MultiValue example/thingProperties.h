#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>

// Set the Thing Id value
const char THING_ID[] = "";

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onSwitchChange();
void onColorChange();

bool Switch;
CloudLocation Loc;
CloudColor Color;

void initProperties(){
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(Switch, READWRITE, ON_CHANGE, onSwitchChange);
  ArduinoCloud.addProperty(Loc, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(Color, READWRITE, ON_CHANGE, onColorChange);
}

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SSID, PASS);
