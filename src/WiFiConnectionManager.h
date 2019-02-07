
#include "ConnectionManager.h"

class WiFiConnectionManager : public ConnectionManager {
public:
  WiFiConnectionManager(const char *ssid, const char *pass);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return wifiClient; };
  

private:
  
  void changeConnectionState(NetworkConnectionState _newState);

  const int CHECK_INTERVAL_INIT = 100;
  const int CHECK_INTERVAL_CONNECTING = 500;
  const int CHECK_INTERVAL_GETTIME = 100;
  const int CHECK_INTERVAL_CONNECTED = 10000;
  const int CHECK_INTERVAL_RETRYING = 5000;
  const int CHECK_INTERVAL_DISCONNECTED = 1000;
  const int CHECK_INTERVAL_ERROR = 500;

  const int MAX_GETTIME_RETRY = 30;

  const char *ssid, *pass;
  unsigned long lastConnectionTickTime, lastNetworkStep;
  unsigned long getTimeRetries;

  WiFiClient wifiClient;
  int connectionTickTimeInterval;
};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

WiFiConnectionManager::WiFiConnectionManager(const char *ssid, const char *pass) :
  ssid(ssid), pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_INIT),
  getTimeRetries(MAX_GETTIME_RETRY) {
}

unsigned long WiFiConnectionManager::getTime() {
  return WiFi.getTime();
}

void WiFiConnectionManager::init() {
}

void WiFiConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  char msgBuffer[120];
  int newInterval = CHECK_INTERVAL_INIT;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      *msgBuffer = 0;
      sprintf(msgBuffer, "Connecting to \"%s\"", ssid);
      debugMessage(msgBuffer, 2);
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_GETTIME:
      newInterval = CHECK_INTERVAL_GETTIME;
      debugMessage("Acquiring Time from Network", 3);
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      *msgBuffer = 0;
      sprintf(msgBuffer, "WiFi.status(): %d", WiFi.status());
      debugMessage(msgBuffer, 4);
      *msgBuffer = 0;
      sprintf(msgBuffer, "Connection to \"%s\" lost.", ssid);
      debugMessage(msgBuffer, 0);
      debugMessage("Attempting reconnection", 0);
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}

void WiFiConnectionManager::check() {
  char msgBuffer[120];
  unsigned long now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_INIT:
        networkStatus = WiFi.status();
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == NETWORK_HARDWARE_ERROR) {
          debugMessage("WiFi Hardware not available\nMake sure you are using a WiFi enabled board/shield", 0);
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Current WiFi Firmware: %s", WiFi.firmwareVersion());
        debugMessage(msgBuffer, 0);
        if(strcmp(WiFi.firmwareVersion(), WIFI_FIRMWARE_VERSION_REQUIRED) != 0){
          *msgBuffer = 0;
          sprintf(msgBuffer, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
          debugMessage(msgBuffer, 0);
          debugMessage("Please update to latest version for optimal performance.", 0);
        }
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        networkStatus = WiFi.begin(ssid, pass);
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 4);
        if (networkStatus != NETWORK_CONNECTED) {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connection to \"%s\" failed", ssid);
          debugMessage(msgBuffer, 0);

          *msgBuffer = 0;
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          //changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connected to \"%s\"", ssid);
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          getTimeRetries = MAX_GETTIME_RETRY;
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        
        unsigned long networkTime;
        networkTime = WiFi.getTime();
        
        debugMessage(".", 3, false, false);
        if(networkTime > lastValidTimestamp){
          lastValidTimestamp = networkTime;
          *msgBuffer = 0;
          sprintf(msgBuffer, "Network Time: %u", networkTime);
          debugMessage(msgBuffer, 3);
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        } else if (WiFi.status() != WL_CONNECTED) {
           changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        } else if (!getTimeRetries--) {
           changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        } 
        break;
      case CONNECTION_STATE_CONNECTED:
        // keep testing connection
        networkStatus = WiFi.status();
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 4);
        if (networkStatus != WL_CONNECTED) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connected to \"%s\"", ssid);
        debugMessage(msgBuffer, 4);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        //WiFi.disconnect();
        WiFi.end();

        
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        //wifiClient.stop();
        break;
    }
    lastConnectionTickTime = now;
  }
}
