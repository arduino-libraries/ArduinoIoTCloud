#include <ArduinoIoTCloud.h>
#include <ConnectionManager.h>

#if defined(BOARD_HAS_WIFI)
  #include <WiFiConnectionManager.h>
#elif defined(BOARD_HAS_GSM)
  #include <GSMConnectionManager.h>
#else
  #error "Arduino IoT Cloud currently only supports MKR1000, MKR WiFi 1010 and MKR GSM 1400"
#endif


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

#if defined(BOARD_HAS_WIFI)
  ConnectionManager * ArduinoIoTPreferredConnection = new WiFiConnectionManager(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  ConnectionManager * ArduinoIoTPreferredConnection = new GSMConnectionManager(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif


