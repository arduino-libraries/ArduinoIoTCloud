#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>
//#include <EthernetConnectionManager.h>


char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
// Your THING_ID
#define THING_ID "ARDUINO_IOT_CLOUD_THING_ID"

void onLedChange();

bool led;
int potentiometer;

void initProperties() {
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(led, READWRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(potentiometer, READ, ON_CHANGE);
}

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_SSID, SECRET_PASS);
//ConnectionManager *ArduinoIoTPreferredConnection = new EthConnectionManager(mac);
