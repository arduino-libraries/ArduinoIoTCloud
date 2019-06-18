/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

/******************************************************************************
   INCLUDES
 ******************************************************************************/

#include <Client.h>
#include <Udp.h>

#include "utility/NTPUtils.h"
#include "utility/DebugUtils.h"

/******************************************************************************
   TYPEDEFS
 ******************************************************************************/

enum class NetworkConnectionState {
  INIT,
  CONNECTING,
  CONNECTED,
  GETTIME,
  DISCONNECTING,
  DISCONNECTED,
  ERROR
};

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class ConnectionManager {
  public:
    virtual void init() = 0;
    virtual void check() = 0;
    virtual unsigned long getTime() = 0;
    virtual Client &getClient();
    virtual UDP &getUDP();

    virtual NetworkConnectionState getStatus() {
      return netConnectionState;
    }

  protected:
    unsigned long lastValidTimestamp = 0;
    NetworkConnectionState netConnectionState = NetworkConnectionState::INIT;

};

#ifdef ARDUINO_SAMD_MKR1000
  #include <WiFi101.h>
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_SHIELD
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_REQUIRED
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT)
  #include <WiFiNINA.h>
  #define BOARD_HAS_WIFI
  #define NETWORK_HARDWARE_ERROR WL_NO_MODULE
  #define NETWORK_IDLE_STATUS WL_IDLE_STATUS
  #define NETWORK_CONNECTED WL_CONNECTED
  #define WIFI_FIRMWARE_VERSION_REQUIRED WIFI_FIRMWARE_LATEST_VERSION
#endif

#ifdef ARDUINO_SAMD_MKRGSM1400
  #include <MKRGSM.h>
  #define BOARD_HAS_GSM
  #define NETWORK_HARDWARE_ERROR GPRS_PING_ERROR
  #define NETWORK_IDLE_STATUS GSM3_NetworkStatus_t::IDLE
  #define NETWORK_CONNECTED GSM3_NetworkStatus_t::GPRS_READY
#endif

#endif /* CONNECTION_MANAGER_H_ */
