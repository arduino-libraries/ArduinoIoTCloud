#ifndef CONNECTION_MANAGER_H_INCLUDED
#define CONNECTION_MANAGER_H_INCLUDED

#include <Client.h>

/* NOT USED FOR NOW */
enum ArduinoIoTConnectionStatus {
  IOT_STATUS_IDLE,/* only at start */
  IOT_STATUS_CLOUD_IDLE,
  IOT_STATUS_CLOUD_CONNECTING,
  IOT_STATUS_CLOUD_CONNECTED,
  IOT_STATUS_CLOUD_DISCONNECTED,
  IOT_STATUS_CLOUD_ERROR,
  IOT_STATUS_NETWORK_IDLE,
  IOT_STATUS_NETWORK_CONNECTED,
  IOT_STATUS_NETWORK_CONNECTING,
  IOT_STATUS_NETWORK_DISCONNECTED,
  IOT_STATUS_NETWORK_ERROR,
  IOT_STATUS_ERROR_GENERIC
};

class ConnectionManager {
public:
  virtual void init() = 0;
  virtual void check() = 0;
  virtual ArduinoIoTConnectionStatus status() = 0;
  virtual unsigned long getTime() = 0;
  virtual Client &getClient();
};

// ********* NETWORK LAYER **********
// max network layer connection retries
#define NETWORK_LAYER_CONNECTION_RETRIES 6
// ms to wait between each retry
#define NETWORK_LAYER_CONNECTION_TIMEOUT 10000
// ms to wait between each retry
#define NETWORK_LAYER_RECONNECTION_TIMEOUT 2000

// ********** CLOUD LAYER *********** 
// max arduino cloud connection retries
#define ARDUINO_IOT_CLOUD_CONNECTION_RETRIES 30
// max wifi connection retries
#define ARDUINO_IOT_CLOUD_CONNECTION_TIMEOUT 3000

// Network Connection Status
//int iotStatus = IOT_STATUS_IDLE;   
//int networkStatus = NETWORK_IDLE_STATUS;

// === NETWORK CONNECTION MANAGEMENT ===
// last time when the Network Connection was checked
//unsigned long lastNetworkCheck = 0;
// time interval to check the Network Connection
//static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;
// timeout between each network connection retry
//static const unsigned long NETWORK_CONNECTION_TIMEOUT = 2000;

#ifdef ARDUINO_SAMD_MKR1000
#include <WiFi101.h>
#include "WiFiConnectionManager.h"
#define BOARD_HAS_WIFI
#define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
#define NETWORK_IDLE_STATUS WL_IDLE_STATUS
#define NETWORK_CONNECTED WL_CONNECTED
#endif

#ifdef ARDUINO_SAMD_MKRWIFI1010
#include <WiFiNINA.h>
#include "WiFiConnectionManager.h"
#define BOARD_HAS_WIFI
#define NETWORK_HARDWARE_ERROR WL_NO_MODULE
#define NETWORK_IDLE_STATUS WL_IDLE_STATUS
#define NETWORK_CONNECTED WL_CONNECTED
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
#include <MKRGSM.h>
#define BOARD_HAS_GSM
#define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
#define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
#define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
#endif

#include <ArduinoIoTCloud.h>

#endif