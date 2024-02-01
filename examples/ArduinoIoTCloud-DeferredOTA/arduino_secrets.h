#include <Arduino_ConnectionHandler.h>

/* A complete list of supported boards with WiFi is available here:
 * https://github.com/arduino-libraries/ArduinoIoTCloud/#what
 */
#if defined(BOARD_HAS_WIFI)
  #define SECRET_SSID "YOUR_WIFI_NETWORK_NAME"
  #define SECRET_PASS "YOUR_WIFI_PASSWORD"
#endif

/* ESP8266 ESP32*/
#if defined(BOARD_HAS_SECRET_KEY)
  #define SECRET_DEVICE_KEY "my-device-password"
#endif

/* Portenta H7 + Ethernet shield */
#if defined(BOARD_HAS_ETHERNET)
  #define SECRET_OPTIONAL_IP ""
  #define SECRET_OPTIONAL_DNS ""
  #define SECRET_OPTIONAL_GATEWAY ""
  #define SECRET_OPTIONAL_NETMASK ""
#endif
