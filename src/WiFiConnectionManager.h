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

#ifndef WIFI_CONNECTION_MANAGER_H_
#define WIFI_CONNECTION_MANAGER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "ConnectionManager.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

#include <WiFiUdp.h>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class WiFiConnectionManager : public ConnectionManager {
  public:
    WiFiConnectionManager(const char *ssid, const char *pass);

    virtual void init();
    virtual unsigned long getTime();
    virtual void check();
    virtual Client &getClient() {
      return wifiClient;
    };
    virtual UDP &getUDP() {
      return udp;
    };

    WiFiUDP udp;

  private:

    void changeConnectionState(NetworkConnectionState const newState);

    const unsigned long CHECK_INTERVAL_IDLE = 100;
    const unsigned long CHECK_INTERVAL_INIT = 100;
    const unsigned long CHECK_INTERVAL_CONNECTING = 500;
    const unsigned long CHECK_INTERVAL_CONNECTED = 10000;
    const unsigned long CHECK_INTERVAL_RETRYING = 5000;
    const unsigned long CHECK_INTERVAL_DISCONNECTED = 1000;
    const unsigned long CHECK_INTERVAL_ERROR = 500;

    const char *ssid, *pass;
    unsigned long lastConnectionTickTime;

    WiFiClient wifiClient;
    unsigned long connectionTickTimeInterval;
};

#endif /* #ifdef BOARD_HAS_WIFI */

#endif /* WIFI_CONNECTION_MANAGER_H_ */
