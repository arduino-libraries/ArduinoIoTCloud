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
  ConnectionManager * ArduinoIoTPreferredConnection = new WiFiConnectionManager(WIFI_SECRET_SSID, WIFI_SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  ConnectionManager * ArduinoIoTPreferredConnection = new GSMConnectionManager(GSM_SECRET_PIN, GSM_SECRET_APN, GSM_SECRET_LOGIN, GSM_SECRET_PASS);
#endif


