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

/* MKR GSM 1400 */ /* MKR NB 1500 */ /* Portenta CAT.M1/NB IoT GNSS Shield */
#if defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB) || defined(BOARD_HAS_CATM1_NBIOT)
  #define SECRET_PIN ""
  #define SECRET_APN ""
  #define SECRET_LOGIN ""
  #define SECRET_PASS ""
#endif

/* MKR WAN 1300/1310 */
#if defined(BOARD_HAS_LORA)
  #define SECRET_APP_EUI ""
  #define SECRET_APP_KEY ""
#endif

/* Portenta H7 + Ethernet shield */
#if defined(BOARD_HAS_ETHERNET)
  #define SECRET_OPTIONAL_IP ""
  #define SECRET_OPTIONAL_DNS ""
  #define SECRET_OPTIONAL_GATEWAY ""
  #define SECRET_OPTIONAL_NETMASK ""
#endif
