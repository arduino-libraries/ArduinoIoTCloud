#include <Arduino_ConnectionHandler.h>

/* If provided, the Wi-Fi Credentials will be passed along to the Notecard. If
 * the Notecard supports Wi-Fi, it will attempt to connect to the network using
 * these credentials, if not, the Notecard will safely ignore these values. */
#define SECRET_WIFI_SSID "YOUR_WIFI_NETWORK_NAME"
#define SECRET_WIFI_PASS "YOUR_WIFI_PASSWORD"
