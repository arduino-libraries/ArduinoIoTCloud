#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "aws_config.h"

#if !(defined(BOARD_STM32H7))
  #error "Please check Arduino IoT Cloud supported boards list: https://github.com/arduino-libraries/ArduinoIoTCloud/#what"
#endif

void onLedChange();

bool led;
int potentiometer;
int seconds;

void initProperties() {
  ArduinoCloud.addProperty(led, Permission::Write).onUpdate(onLedChange);
  ArduinoCloud.addProperty(potentiometer, Permission::Read).publishOnChange(10);
  ArduinoCloud.addProperty(seconds, Permission::Read).publishOnChange(1);
}

//#define USE_ETHERNET_DHCP_CONNECTION
//#define USE_ETHERNET_MANUAL_CONNECTION
#define USE_WIFI_CONNECTION
//#define USE_CATM1_NBIOT_CONNECTION

#include "arduino_secrets.h"

#if defined(BOARD_HAS_ETHERNET) && defined(USE_ETHERNET_CONNECTION)
  /* DHCP mode */
  EthernetConnectionHandler ArduinoIoTPreferredConnection;
  EthernetConnectionHandler AWSIoTPreferredConnection;
#elif defined(BOARD_HAS_ETHERNET) && defined(USE_ETHERNET_MANUAL_CONNECTION)
  /* Manual mode. It will fallback in DHCP mode if SECRET_OPTIONAL_IP is invalid or equal to "0.0.0.0" */
  EthernetConnectionHandler ArduinoIoTPreferredConnection(SECRET_ETH_OPTIONAL_IP, SECRET_ETH_OPTIONAL_DNS, SECRET_ETH_OPTIONAL_GATEWAY, SECRET_ETH_OPTIONAL_NETMASK);
  EthernetConnectionHandler AWSIoTPreferredConnection(SECRET_ETH_OPTIONAL_IP, SECRET_ETH_OPTIONAL_DNS, SECRET_ETH_OPTIONAL_GATEWAY, SECRET_ETH_OPTIONAL_NETMASK);
#elif defined(BOARD_HAS_WIFI) && defined(USE_WIFI_CONNECTION)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
  WiFiConnectionHandler AWSIoTPreferredConnection(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
#elif defined(BOARD_HAS_CATM1_NBIOT) && defined(USE_CATM1_NBIOT_CONNECTION)
  CatM1ConnectionHandler ArduinoIoTPreferredConnection(SECRET_CATM_PIN, SECRET_CATM_APN, SECRET_CATM_LOGIN, SECRET_CATM_PASS);
  CatM1ConnectionHandler AWSIoTPreferredConnection(SECRET_CATM_PIN, SECRET_CATM_APN, SECRET_CATM_LOGIN, SECRET_CATM_PASS);
#endif

BearSSLClient sslClientAWS(AWSIoTPreferredConnection.getClient());
MqttClient mqttClientAWS(sslClientAWS);
