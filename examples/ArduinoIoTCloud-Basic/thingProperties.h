#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h"

#if !(defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_LORA) || \
      defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET) || defined(BOARD_HAS_CATM1_NBIOT))
  #error "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if defined(BOARD_HAS_SECRET_KEY)
  #define BOARD_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

void initProperties() {
#if defined(BOARD_HAS_SECRET_KEY)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET) || defined(BOARD_HAS_CATM1_NBIOT)
  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);
#elif defined(BOARD_HAS_LORA)
  ArduinoCloud.addProperty(led, 1, READWRITE, ON_CHANGE, onLedChange);
  ArduinoCloud.addProperty(potentiometer, 2, READ, ON_CHANGE);
  ArduinoCloud.addProperty(seconds, 3, READ, 5 * MINUTES);
#endif
}

#if defined(BOARD_HAS_ETHERNET)
  /* DHCP mode */
  //EthernetConnectionHandler ArduinoIoTPreferredConnection;
  /* Manual mode. It will fallback in DHCP mode if SECRET_OPTIONAL_IP is invalid or equal to "0.0.0.0" */
  EthernetConnectionHandler ArduinoIoTPreferredConnection(SECRET_OPTIONAL_IP, SECRET_OPTIONAL_DNS, SECRET_OPTIONAL_GATEWAY, SECRET_OPTIONAL_NETMASK);
#elif defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_LORA)
  LoRaConnectionHandler ArduinoIoTPreferredConnection(SECRET_APP_EUI, SECRET_APP_KEY, _lora_band::EU868, NULL, _lora_class::CLASS_A);
#elif defined(BOARD_HAS_NB)
  NBConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_CATM1_NBIOT)
  CatM1ConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif
