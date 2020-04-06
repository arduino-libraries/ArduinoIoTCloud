#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#define THING_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#define BOARD_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"

void onSwitchButtonChange();
void onColorChange();

bool switchButton;
CloudLocation location;
CloudColor color;

void initProperties() {
#if defined(BOARD_ESP8266)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
  ArduinoCloud.setThingId(THING_ID);
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB)
  ArduinoCloud.addProperty(switchButton, WRITE, ON_CHANGE, onSwitchButtonChange);
  ArduinoCloud.addProperty(location, READ, ON_CHANGE);
  ArduinoCloud.addProperty(color, READWRITE, ON_CHANGE, onColorChange);
#elif defined(BOARD_HAS_LORA)
  ArduinoCloud.addProperty(switchButton, 1, WRITE, ON_CHANGE, onSwitchButtonChange);
  ArduinoCloud.addProperty(location, 2, READ, ON_CHANGE);
  ArduinoCloud.addProperty(color, 3, READWRITE, ON_CHANGE, onColorChange);
#endif
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_LORA)
  LoRaConnectionHandler ArduinoIoTPreferredConnection(SECRET_APP_EUI, SECRET_APP_KEY, _lora_band::EU868, _lora_class::CLASS_A);
#elif defined(BOARD_HAS_NB)
  NBConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif
