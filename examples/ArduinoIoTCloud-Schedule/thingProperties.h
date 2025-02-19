#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h"

#if !defined(HAS_TCP)
  #error  "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if !defined(BOARD_HAS_SECURE_ELEMENT)
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
#if defined(HAS_TCP)
  ArduinoCloud.addProperty(switchButton, Permission::Write);
  ArduinoCloud.addProperty(oneShot, Permission::ReadWrite);
  ArduinoCloud.addProperty(minute, Permission::ReadWrite);
  ArduinoCloud.addProperty(hourly, Permission::ReadWrite);
  ArduinoCloud.addProperty(daily, Permission::ReadWrite);
  ArduinoCloud.addProperty(weekly, Permission::ReadWrite);
  ArduinoCloud.addProperty(monthly, Permission::ReadWrite);
  ArduinoCloud.addProperty(yearly, Permission::ReadWrite);

#if !defined(BOARD_HAS_SECURE_ELEMENT)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif
#endif
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
#elif defined(BOARD_HAS_GSM)
  GSMConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_NB)
  NBConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_CATM1_NBIOT)
  CatM1ConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#elif defined(BOARD_HAS_ETHERNET)
  /* DHCP mode */
  //EthernetConnectionHandler ArduinoIoTPreferredConnection;
  /* Manual mode. It will fallback in DHCP mode if SECRET_OPTIONAL_IP is invalid or equal to "0.0.0.0" */
  EthernetConnectionHandler ArduinoIoTPreferredConnection(SECRET_OPTIONAL_IP, SECRET_OPTIONAL_DNS, SECRET_OPTIONAL_GATEWAY, SECRET_OPTIONAL_NETMASK);
#elif defined(BOARD_HAS_CELLULAR)
  CellularConnectionHandler ArduinoIoTPreferredConnection(SECRET_PIN, SECRET_APN, SECRET_LOGIN, SECRET_PASS);
#endif
