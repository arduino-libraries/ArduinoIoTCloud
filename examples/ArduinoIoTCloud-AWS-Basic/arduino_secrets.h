#include <Arduino_ConnectionHandler.h>

/* A complete list of supported boards with WiFi is available here:
 * https://github.com/arduino-libraries/ArduinoIoTCloud/#what
 */
#if defined(BOARD_HAS_WIFI) && defined(USE_WIFI_CONNECTION)
  #define SECRET_WIFI_SSID ""
  #define SECRET_WIFI_PASS ""
#endif

/* Portenta H7 + Ethernet shield */
#if defined(BOARD_HAS_ETHERNET) && defined(USE_ETHERNET_MANUAL_CONNECTION)
  #define SECRET_ETH_OPTIONAL_IP ""
  #define SECRET_ETH_OPTIONAL_DNS ""
  #define SECRET_ETH_OPTIONAL_GATEWAY ""
  #define SECRET_ETH_OPTIONAL_NETMASK ""
#endif

/* Portenta CAT.M1/NB IoT GNSS Shield */
#if defined(BOARD_HAS_CATM1_NBIOT) && defined(USE_CATM1_NBIOT_CONNECTION)
  #define SECRET_CATM_PIN ""
  #define SECRET_CATM_APN ""
  #define SECRET_CATM_LOGIN ""
  #define SECRET_CATM_PASS ""
#endif
