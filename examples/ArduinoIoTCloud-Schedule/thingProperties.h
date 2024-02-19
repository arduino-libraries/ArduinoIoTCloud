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

void onSwitchButtonChange();

bool switchButton;
CloudSchedule oneShot;
CloudSchedule minute;
CloudSchedule hourly;
CloudSchedule daily;
CloudSchedule weekly;
CloudSchedule monthly;
CloudSchedule yearly;

void initProperties() {
#if defined(BOARD_HAS_SECRET_KEY)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_ETHERNET) || defined(BOARD_HAS_CATM1_NBIOT)
  ArduinoCloud.addProperty(switchButton, WRITE, ON_CHANGE);
  ArduinoCloud.addProperty(oneShot, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(minute, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(hourly, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(daily, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(weekly, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(monthly, READWRITE, ON_CHANGE);
  ArduinoCloud.addProperty(yearly, READWRITE, ON_CHANGE);
#elif defined(BOARD_HAS_LORA)
  ArduinoCloud.addProperty(switchButton, 1, WRITE, ON_CHANGE);
#endif
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_LORA)
  LoRaConnectionHandler ArduinoIoTPreferredConnection(SECRET_APP_EUI, SECRET_APP_KEY, _lora_band::EU868, NULL, _lora_class::CLASS_A);
#elif defined(BOARD_HAS_NB)
  NBConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_CATM1_NBIOT)
  CatM1ConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_ETHERNET)
  /* DHCP mode */
  //EthernetConnectionHandler ArduinoIoTPreferredConnection;
  /* Manual mode. It will fallback in DHCP mode if SECRET_OPTIONAL_IP is invalid or equal to "0.0.0.0" */
  EthernetConnectionHandler ArduinoIoTPreferredConnection(SECRET_OPTIONAL_IP, SECRET_OPTIONAL_DNS, SECRET_OPTIONAL_GATEWAY, SECRET_OPTIONAL_NETMASK);
#endif
