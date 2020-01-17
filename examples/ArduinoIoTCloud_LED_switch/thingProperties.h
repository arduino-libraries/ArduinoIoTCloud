#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#if defined(BOARD_HAS_WIFI)
#elif defined(BOARD_HAS_GSM)
#elif defined(BOARD_HAS_LORA)
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
  #if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM)
  ArduinoCloud.addProperty(led, READWRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(potentiometer, READ, ON_CHANGE);
  #elif defined(BOARD_HAS_LORA)
  ArduinoCloud.addProperty(led, 1, READWRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(potentiometer, 2, READ, ON_CHANGE);
  #endif

}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_LORA)
  LoRaConnectionHandler ArduinoIoTPreferredConnection(SECRET_APP_EUI, SECRET_APP_KEY, _lora_band::EU868, _lora_class::CLASS_A);
#endif
