#include <ArduinoIoTCloud.h>
/*
 The following include line is used for WiFi enabled boards (MKR1000, MKR WiFi 1010)
*/
#include <WiFiConnectionManager.h>
/*
 If you prefer using a MKR GSM 1400 comment the line above and uncommet the following.
*/
//#include <GSMConnectionManager.h>

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

/*
 The following include line is used for WiFi enabled boards (MKR1000, MKR WiFi 1010)
*/
ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_SSID, SECRET_PASS);
/*
 If you prefer using a MKR GSM 1400 comment the line above and uncommet the following.
*/
//ConnectionManager *ArduinoIoTPreferredConnection = new GSMConnectionManager(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
