#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h"

#if !(defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_ETHERNET))
  #error "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

#if defined(BOARD_HAS_SECRET_KEY)
  #define BOARD_ID "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
#endif

void onLedChange();

bool led;

void initProperties() {
#if defined(BOARD_HAS_SECRET_KEY)
  ArduinoCloud.setBoardId(BOARD_ID);
  ArduinoCloud.setSecretDeviceKey(SECRET_DEVICE_KEY);
#endif

  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#elif defined(BOARD_HAS_ETHERNET)
  /* DHCP mode */
  //EthernetConnectionHandler ArduinoIoTPreferredConnection;
  /* Manual mode. It will fallback in DHCP mode if SECRET_OPTIONAL_IP is invalid or equal to "0.0.0.0" */
  EthernetConnectionHandler ArduinoIoTPreferredConnection(SECRET_OPTIONAL_IP, SECRET_OPTIONAL_DNS, SECRET_OPTIONAL_GATEWAY, SECRET_OPTIONAL_NETMASK);
#endif
