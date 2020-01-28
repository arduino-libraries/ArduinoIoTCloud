#include <Arduino_ConnectionHandler.h>

/* MKR1000, MKR WiFi 1010 */
#if defined(BOARD_HAS_WIFI)
  #define SECRET_SSID "YOUR_WIFI_NETWORK_NAME"
  #define SECRET_PASS "YOUR_WIFI_PASSWORD"
#endif

/* MKR GSM 1400 */
#if defined(BOARD_HAS_GSM)
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

