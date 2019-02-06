/*
 * This file is part of ArduinoIoTCloud.
 *
 * Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
 *
 * This software is released under the GNU General Public License version 3,
 * which covers the main part of arduino-cli.
 * The terms of this license can be found at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * You can be released from the requirements of the above licenses by purchasing
 * a commercial license. Buying such a license is mandatory if you want to modify or
 * otherwise use the software for commercial activities involving the Arduino
 * software without disclosing the source code of your own applications. To purchase
 * a commercial license, send an email to license@arduino.cc.
 */

#include <MKRGSM.h>
#include "ConnectionManager.h"

class GSMConnectionManager : public ConnectionManager {
public:
  GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return networkClient; };

  GSMClient networkClient;
  GSM gsmAccess;
  GPRS gprs;
private:
  const char *pin, *apn, *login, *pass;
  unsigned long lastNetworkCheck, lastNetworkStep;
};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

GSMConnectionManager::GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass) :
  pin(pin), apn(apn), login(login), pass(pass), lastNetworkCheck(millis()) {
}

unsigned long GSMConnectionManager::getTime() {
  return gsmAccess.getTime();
}

void GSMConnectionManager::init() {
  if (gsmAccess.begin(pin) == GSM_READY && gprs.attachGPRS(apn, login, pass) == GPRS_READY) {
    Serial.println("GPRS connected");
    int pingResult = gprs.ping("google.com");
    if (pingResult >= 0) {
      Serial.print("SUCCESS! RTT = ");
      Serial.print(pingResult);
      Serial.println(" ms");
    }
  } else {
    Serial.println("GPRS not connected");
    while(1);
  }
}

void GSMConnectionManager::check() {
  if (millis() - lastNetworkCheck < NETWORK_CONNECTION_INTERVAL) {
    return;
  }

  Serial.print("<<Network Status: ");
  if (gprs.attachGPRS(apn, login, pass) == GPRS_READY) {
    Serial.println("CONNECTED");
    lastNetworkCheck = millis();
  } else {
    Serial.println("NOT CONNECTED");
  }
}
