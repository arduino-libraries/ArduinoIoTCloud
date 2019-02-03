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
